// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <iterator>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ByteLiterals.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(SubchainJobs job) noexcept -> std::string_view
{
    try {
        using Job = SubchainJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::filter, "filter"},
            {Job::mempool, "mempool"},
            {Job::block, "block"},
            {Job::prepare_reorg, "prepare_reorg"},
            {Job::update, "update"},
            {Job::process, "process"},
            {Job::watchdog, "watchdog"},
            {Job::watchdog_ack, "watchdog_ack"},
            {Job::init, "init"},
            {Job::key, "key"},
            {Job::prepare_shutdown, "prepare_shutdown"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)(": invalid SubchainJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::Subaccount& subaccount,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    const std::string_view parent,
    CString&& fromChildren,
    CString&& toChildren,
    CString&& toScan,
    CString&& toProgress,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          describe(subaccount, subchain, alloc),
          0ms,
          batch,
          alloc,
          {
              {CString{parent, alloc}, Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
          },
          {},
          {
              {SocketType::Publish,
               {
                   {toChildren, Direction::Bind},
               }},
          })
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_oracle_(mempool)
    , owner_(subaccount.Parent().NymID())
    , account_type_(subaccount.Type())
    , id_(subaccount.ID())
    , subchain_(subchain)
    , chain_(node_.Chain())
    , filter_type_(filter)
    , db_key_(db.GetSubchainID(id_, subchain_))
    , null_position_(make_blank<block::Position>::value(api_))
    , genesis_(node_.HeaderOracle().GetPosition(0))
    , from_ssd_endpoint_(std::move(toChildren))
    , to_ssd_endpoint_(std::move(fromChildren))
    , to_index_endpoint_(network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_scan_endpoint_(std::move(toScan))
    , to_rescan_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_process_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_progress_endpoint_(std::move(toProgress))
    , shutdown_endpoint_(parent, alloc)
    , element_cache_(
          db_.GetPatterns(db_key_, alloc.resource()),
          db_.GetUnspentOutputs(id_, subchain_, alloc.resource()),
          alloc)
    , to_children_(pipeline_.Internal().ExtraSocket(0))
    , pending_state_(State::normal)
    , state_(State::normal)
    , reorgs_(alloc)
    , progress_(std::nullopt)
    , rescan_(std::nullopt)
    , index_(std::nullopt)
    , process_(std::nullopt)
    , scan_(std::nullopt)
    , have_children_(false)
    , child_activity_({
          {JobType::scan, {}},
          {JobType::process, {}},
          {JobType::index, {}},
          {JobType::rescan, {}},
          {JobType::progress, {}},
      })
    , watchdog_(api_.Network().Asio().Internal().GetTimer())
{
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::Subaccount& subaccount,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    const std::string_view parent,
    allocator_type alloc) noexcept
    : SubchainStateData(
          api,
          node,
          db,
          mempool,
          subaccount,
          filter,
          subchain,
          batch,
          parent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          alloc)
{
}

auto SubchainStateData::ChangeState(
    const State state,
    StateSequence reorg) noexcept -> bool
{
    if (auto old = pending_state_.exchange(state); old == state) {

        return true;
    }

    auto lock = lock_for_reorg(name_, reorg_lock_);
    auto output{false};

    switch (state) {
        case State::normal: {
            if (State::reorg != state_) { break; }

            transition_state_normal();
            output = true;
        } break;
        case State::reorg: {
            if (State::shutdown == state_) { break; }

            transition_state_reorg(reorg);
            output = true;
        } break;
        case State::shutdown: {
            if (State::reorg == state_) { break; }

            transition_state_shutdown();
            output = true;
        } break;
        default: {
            OT_FAIL;
        }
    }

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())(name_)(" failed to change state from ")(
            print(state_))(" to ")(print(state))
            .Flush();

        OT_FAIL;
    }

    return output;
}

auto SubchainStateData::clear_children() noexcept -> void
{
    if (have_children_) {
        auto rc = scan_->ChangeState(JobState::shutdown, {});

        OT_ASSERT(rc);

        rc = process_->ChangeState(JobState::shutdown, {});

        OT_ASSERT(rc);

        rc = index_->ChangeState(JobState::shutdown, {});

        OT_ASSERT(rc);

        rc = rescan_->ChangeState(JobState::shutdown, {});

        OT_ASSERT(rc);

        rc = progress_->ChangeState(JobState::shutdown, {});

        OT_ASSERT(rc);

        scan_.reset();
        process_.reset();
        index_.reset();
        rescan_.reset();
        progress_.reset();
        have_children_ = false;
    }
}

auto SubchainStateData::describe(
    const crypto::Subaccount& account,
    const Subchain subchain,
    allocator_type alloc) noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << account.Describe();
    out << ' ';
    out << print(subchain);
    out << " subchain";

    return CString{alloc} + out.str().c_str();
}

auto SubchainStateData::do_reorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position ancestor) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" processing reorg to ")(print(ancestor))
        .Flush();
    const auto tip = db_.SubchainLastScanned(db_key_);
    // TODO use ancestor
    const auto& headers = node_.HeaderOracle();

    try {
        const auto reorg =
            headers.Internal().CalculateReorg(headerOracleLock, tip);

        if (0u == reorg.size()) {
            log_(OT_PRETTY_CLASS())(name_)(
                " no action required for this subchain")
                .Flush();

            return;
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" ")(reorg.size())(
                " previously mined blocks have been invalidated")
                .Flush();
        }

        if (db_.ReorgTo(
                headerOracleLock,
                tx,
                headers,
                id_,
                subchain_,
                db_key_,
                reorg)) {
            scan_->ProcessReorg(ancestor);
            process_->ProcessReorg(ancestor);
            index_->ProcessReorg(ancestor);
            rescan_->ProcessReorg(ancestor);
            progress_->ProcessReorg(ancestor);
        } else {

            ++errors;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            name_)(" header oracle claims existing tip ")(print(tip))(
            " is invalid")
            .Flush();
        ++errors;
    }
}

auto SubchainStateData::do_shutdown() noexcept -> void { clear_children(); }

auto SubchainStateData::do_startup() noexcept -> void
{
    auto me = shared_from_this();
    progress_.emplace(me);
    rescan_.emplace(me);
    index_.emplace(get_index(me));
    process_.emplace(me);
    scan_.emplace(me);
    have_children_ = true;
    const auto now = Clock::now();

    for (auto& [type, time] : child_activity_) { time = now; }

    do_work();
}

auto SubchainStateData::get_account_targets(
    const Elements& elements,
    alloc::Resource* alloc) const noexcept -> Targets
{
    auto out = Targets{alloc};
    get_targets(elements, out);

    return out;
}

auto SubchainStateData::get_targets(const Elements& in, Targets& targets)
    const noexcept -> void
{
    targets.reserve(in.size());

    for (const auto& element : in.elements_20_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_32_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_33_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_64_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_65_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    get_targets(in.txos_, targets);
}

auto SubchainStateData::get_targets(const TXOs& utxos, Targets& targets)
    const noexcept -> void
{
    switch (filter_type_) {
        case cfilter::Type::Basic_BCHVariant:
        case cfilter::Type::ES: {
            for (const auto& [outpoint, output] : utxos) {
                targets.emplace_back(outpoint.Bytes());
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        default: {
        }
    }
}

auto SubchainStateData::IndexElement(
    const cfilter::Type type,
    const blockchain::crypto::Element& input,
    const Bip32Index index,
    WalletDatabase::ElementMap& output) const noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" element ")(
        index)(" extracting filter matching patterns")
        .Flush();
    auto& list = output[index];
    const auto scripts = supported_scripts(input);

    switch (type) {
        case cfilter::Type::ES: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                for (const auto& element : e) {
                    list.emplace_back(
                        space(element, list.get_allocator().resource()));
                }
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant:
        default: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto SubchainStateData::Init(boost::shared_ptr<SubchainStateData> me) noexcept
    -> void
{
    signal_startup(me);
}

auto SubchainStateData::match(
    const std::string_view procedure,
    const Log& log,
    const block::Position& position,
    const BlockTarget& targets,
    const GCS& cfilter,
    wallet::ElementCache::Results& resultMap) const noexcept -> bool
{
    const auto GetKeys = [&](const auto& selected) {
        auto& input = selected.first;
        auto out = Set<Bip32Index>{input.get_allocator()};
        const auto start = selected.second.begin();

        for (const auto& match : cfilter.Match(selected.second)) {
            const auto index = std::distance(start, match);

            OT_ASSERT(0 <= index);

            const auto i = static_cast<std::size_t>(index);

            out.emplace(input.at(i));
        }

        return out;
    };
    const auto GetOutpoints = [&](const auto& selected) {
        auto& input = selected.first;
        auto out = Set<block::Outpoint>{input.get_allocator()};
        const auto start = selected.second.begin();

        for (const auto& match : cfilter.Match(selected.second)) {
            const auto index = std::distance(start, match);

            OT_ASSERT(0 <= index);

            const auto i = static_cast<std::size_t>(index);

            out.emplace(input.at(i));
        }

        return out;
    };
    const auto GetResults = [&](const auto& cb,
                                const auto& selected,
                                auto& clean,
                                auto& dirty,
                                auto& output) {
        const auto matches = cb(selected);

        for (const auto& index : selected.first) {
            if (0u == matches.count(index)) {
                clean.emplace(index);
            } else {
                dirty.emplace(index);
            }
        }

        output.first += matches.size();
        output.second += selected.first.size();
    };

    const auto& selected = targets.second;
    const auto& [s20, s32, s33, s64, s65, stxo] = selected;
    const auto matchedTxo = GetOutpoints(stxo);
    auto& results = resultMap[position];
    auto output = std::pair<std::size_t, std::size_t>{};
    GetResults(
        GetKeys,
        s20,
        results.confirmed_no_match_.match_20_,
        results.confirmed_match_.match_20_,
        output);
    GetResults(
        GetKeys,
        s32,
        results.confirmed_no_match_.match_32_,
        results.confirmed_match_.match_32_,
        output);
    GetResults(
        GetKeys,
        s33,
        results.confirmed_no_match_.match_33_,
        results.confirmed_match_.match_33_,
        output);
    GetResults(
        GetKeys,
        s64,
        results.confirmed_no_match_.match_64_,
        results.confirmed_match_.match_64_,
        output);
    GetResults(
        GetKeys,
        s65,
        results.confirmed_no_match_.match_65_,
        results.confirmed_match_.match_65_,
        output);
    GetResults(
        GetOutpoints,
        stxo,
        results.confirmed_no_match_.match_txo_,
        results.confirmed_match_.match_txo_,
        output);
    const auto& [count, of] = output;
    log(OT_PRETTY_CLASS())(name_)(" GCS ")(procedure)(" for block ")(
        print(position))(" matched ")(count)(" of ")(of)(" target elements")
        .Flush();

    return 0u < count;
}

auto SubchainStateData::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::shutdown: {
            shutdown_actor();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto SubchainStateData::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    transition_state_reorg(body.at(1).as<StateSequence>());
}

auto SubchainStateData::process_watchdog_ack(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    child_activity_.at(body.at(1).as<JobType>()) = Clock::now();
}

auto SubchainStateData::ProcessBlock(
    const block::Position& position,
    const block::bitcoin::Block& block) const noexcept -> void
{
    const auto start = Clock::now();
    const auto& name = name_;
    const auto& type = filter_type_;
    const auto& node = node_;
    const auto& filters = node.FilterOracleInternal();
    const auto& blockHash = position.second;
    auto buf = std::array<std::byte, 16_KiB>{};
    auto alloc = alloc::BoostMonotonic{
        buf.data(),
        buf.size(),
        alloc::standard_to_boost(get_allocator().resource())};
    const auto elements = element_cache_.lock_shared()->Get(&alloc);
    auto patterns = std::make_pair(Patterns{&alloc}, Patterns{&alloc});
    select_matches(position, elements, patterns);
    const auto haveTargets = Clock::now();
    const auto cfilter = filters.LoadFilter(type, blockHash, get_allocator());

    OT_ASSERT(cfilter.IsValid());

    const auto haveFilter = Clock::now();
    const auto& [outpoint, key] = patterns;
    const auto confirmed = block.Internal().FindMatches(type, outpoint, key);
    const auto haveMatches = Clock::now();
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    const auto haveHeader = Clock::now();
    handle_confirmed_matches(block, position, confirmed);
    const auto handledMatches = Clock::now();
    const auto& log = log_;
    log(OT_PRETTY_CLASS())(name)(" block ")(print(position))(" processed in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(general.size())(" of ")(key.size())(
        " potential key matches confirmed.")
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(utxo.size())(" of ")(outpoint.size())(
        " potential utxo matches confirmed.")
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load match targets: ")(
        std::chrono::nanoseconds{haveTargets - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load filter: ")(
        std::chrono::nanoseconds{haveFilter - haveTargets})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to find matches: ")(
        std::chrono::nanoseconds{haveMatches - haveFilter})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load block header: ")(
        std::chrono::nanoseconds{haveHeader - haveMatches})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to handle matches: ")(
        std::chrono::nanoseconds{handledMatches - haveHeader})
        .Flush();
}

auto SubchainStateData::ProcessTransaction(
    const block::bitcoin::Transaction& tx) const noexcept -> void
{
    auto buf = std::array<std::byte, 4_KiB>{};
    auto alloc = alloc::BoostMonotonic{
        buf.data(),
        buf.size(),
        alloc::standard_to_boost(get_allocator().resource())};
    const auto elements =
        element_cache_.lock_shared()->Get(get_allocator().resource());
    const auto targets = get_account_targets(elements, &alloc);
    const auto patterns = to_patterns(elements, &alloc);
    const auto parsed = block::ParsedPatterns{patterns};
    const auto outpoints = [&]() {
        auto out = SubchainStateData::Patterns{&alloc};
        translate(elements.txos_, out);

        return out;
    }();
    auto copy = tx.clone();

    OT_ASSERT(copy);

    const auto matches =
        copy->Internal().FindMatches(filter_type_, outpoints, parsed);
    handle_mempool_matches(matches, std::move(copy));
}

auto SubchainStateData::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& ancestor) noexcept -> void
{
    do_reorg(headerOracleLock, tx, errors, ancestor);
}

auto SubchainStateData::ReportScan(const block::Position& pos) const noexcept
    -> void
{
    api_.Crypto().Blockchain().Internal().ReportScan(
        chain_, owner_, account_type_, id_, subchain_, pos);
}

auto SubchainStateData::reorg_children() const noexcept -> std::size_t
{
    return 1u;
}

auto SubchainStateData::Rescan(
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out) const noexcept -> std::optional<block::Position>
{
    return scan(true, best, stop, highestTested, out);
}

auto SubchainStateData::Scan(
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out) const noexcept -> std::optional<block::Position>
{
    return scan(false, best, stop, highestTested, out);
}

auto SubchainStateData::scan(
    const bool rescan,
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out) const noexcept -> std::optional<block::Position>
{
    using namespace std::literals;
    const auto procedure = rescan ? "rescan"sv : "scan"sv;
    const auto& log = log_;
    const auto& name = name_;
    const auto& node = node_;
    const auto& type = filter_type_;
    const auto& headers = node.HeaderOracle();
    const auto& filters = node.FilterOracleInternal();
    const auto start = Clock::now();
    const auto elements =
        element_cache_.lock_shared()->Get(get_allocator().resource());
    constexpr auto GetBatchSize = [](std::size_t in) {
        constexpr auto max = std::size_t{10000};
        constexpr auto min = std::size_t{100};
        static_assert(max > min);

        if (in > (max - min)) {

            return min;
        } else {

            return (max - in);
        }
    };
    static_assert(GetBatchSize(0) == 10000);
    static_assert(GetBatchSize(10000) == 100);
    static_assert(GetBatchSize(9900) == 100);
    static_assert(GetBatchSize(9899) == 101);
    const auto scanBatch = GetBatchSize(elements.size());
    const auto startHeight = highestTested.first + 1;
    const auto stopHeight = std::min(
        std::min<block::Height>(startHeight + scanBatch - 1, best.first), stop);
    auto atLeastOnce{false};
    auto highestClean = std::optional<block::Position>{std::nullopt};

    if (startHeight > stopHeight) {
        log(OT_PRETTY_CLASS())(name)(" attempted to ")(
            procedure)(" filters from ")(startHeight)(" to ")(
            stopHeight)(" but this is impossible")
            .Flush();

        return std::nullopt;
    }

    log(OT_PRETTY_CLASS())(name)(" ")(procedure)("ning filters from ")(
        startHeight)(" to ")(stopHeight)
        .Flush();
    const auto target = static_cast<std::size_t>(stopHeight - startHeight + 1);
    auto* upstream = alloc::standard_to_boost(get_allocator().resource());
    const auto allocBytes =
        (scanBatch * (sizeof(block::Hash) + sizeof(GCS))) + 4_KiB;
    auto alloc = alloc::BoostMonotonic{allocBytes, upstream};
    const auto blocks = headers.BestHashes(startHeight, target, &alloc);
    auto resultMap = wallet::ElementCache::Results{&alloc};
    auto selected = BlockTargets{&alloc};
    select_targets(blocks, elements, startHeight, selected);
    // TODO pre-hash all the selected targets and load cfilters asynchronously
    const auto cfilters = filters.LoadFilters(type, blocks);

    OT_ASSERT(cfilters.size() <= blocks.size());

    auto isClean{true};
    auto s = selected.begin();
    auto f = cfilters.begin();
    auto i = startHeight;

    for (auto end = cfilters.end(); f != end; ++f, ++s, ++i) {
        const auto& blockHash = s->first;
        const auto& cfilter = *f;
        auto testPosition = block::Position{i, blockHash};

        if (blockHash.empty()) {
            LogError()(OT_PRETTY_CLASS())(name)(" empty block hash").Flush();

            break;
        }

        if (false == cfilter.IsValid()) {
            LogError()(OT_PRETTY_CLASS())(name)(" filter for block ")(
                print(testPosition))(" not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        const auto hasMatches =
            match(procedure, log, testPosition, *s, cfilter, resultMap);

        if (hasMatches) {
            isClean = false;
            out.emplace_back(ScanState::dirty, testPosition);
        } else if (isClean) {
            highestClean = testPosition;
        }

        highestTested = std::move(testPosition);
    }

    if (atLeastOnce) {
        if (0u < resultMap.size()) {
            element_cache_.lock()->Add(std::move(resultMap));
        }

        if (rescan && highestClean.has_value()) {
            element_cache_.lock()->Forget(highestClean.value());
        }

        const auto count = out.size();
        log(OT_PRETTY_CLASS())(name)(" ")(procedure)(" found ")(
            count)(" new potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::nanoseconds{Clock::now() - start})
            .Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name)(" ")(procedure)(" interrupted").Flush();
    }

    return highestClean;
}

auto SubchainStateData::select_matches(
    const block::Position& block,
    const Elements& in,
    MatchesToTest& out) const noexcept -> void
{
    const auto subchainID = WalletDatabase::SubchainID{subchain_, id_};
    auto& [outpoint, key] = out;
    auto alloc = outpoint.get_allocator();
    const auto SelectKey =
        [&](const auto& all, const auto& selected, auto& out) {
            if (0u == selected.size()) { return; }

            for (const auto& [index, data] : all) {
                if (0u < selected.count(index)) {
                    out.emplace_back(std::make_pair(
                        std::make_pair(index, subchainID),
                        space(reader(data), alloc.resource())));
                }
            }
        };
    const auto SelectTxo =
        [&](const auto& all, const auto& selected, auto& out) {
            if (0u == selected.size()) { return; }

            const auto self = id_->str();

            for (const auto& [outpoint, pOutput] : all) {
                if (0u < selected.count(outpoint)) {
                    OT_ASSERT(pOutput);

                    for (const auto& key : pOutput->Keys()) {
                        const auto& [id, subchain, index] = key;

                        if (self != id) { continue; }
                        if (subchain_ != subchain) { continue; }

                        out.emplace_back(std::make_pair(
                            std::make_pair(index, subchainID),
                            space(outpoint.Bytes(), alloc.resource())));
                    }
                }
            }
        };

    try {
        const auto& items = in.results_.at(block).confirmed_match_;
        SelectKey(in.elements_20_, items.match_20_, key);
        SelectKey(in.elements_32_, items.match_32_, key);
        SelectKey(in.elements_33_, items.match_33_, key);
        SelectKey(in.elements_64_, items.match_64_, key);
        SelectKey(in.elements_65_, items.match_65_, key);
        SelectTxo(in.txos_, items.match_txo_, outpoint);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(" ")(e.what()).Flush();

        OT_FAIL;
    }
}

auto SubchainStateData::select_targets(
    const BlockHashes& hashes,
    const Elements& in,
    block::Height height,
    BlockTargets& out) const noexcept -> void
{
    for (const auto& hash : hashes) {
        select_targets(block::Position{height++, hash}, in, out);
    }
}

auto SubchainStateData::select_targets(
    const block::Position& block,
    const Elements& in,
    BlockTargets& out) const noexcept -> void
{
    auto alloc = out.get_allocator();
    auto& [hash, selected] = out.emplace_back(std::make_pair(
        block.second,
        std::make_tuple(
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<block::Outpoint>{alloc}, Targets{alloc}))));
    auto& [s20, s32, s33, s64, s65, stxo] = selected;
    s20.first.reserve(in.elements_20_.size());
    s20.second.reserve(in.elements_20_.size());
    s32.first.reserve(in.elements_32_.size());
    s32.second.reserve(in.elements_32_.size());
    s33.first.reserve(in.elements_33_.size());
    s33.second.reserve(in.elements_33_.size());
    s64.first.reserve(in.elements_64_.size());
    s64.second.reserve(in.elements_64_.size());
    s65.first.reserve(in.elements_65_.size());
    s65.second.reserve(in.elements_65_.size());
    stxo.first.reserve(in.txos_.size());
    stxo.second.reserve(in.txos_.size());
    const auto ChooseKey = [](const auto& index, const auto& data, auto& out) {
        out.first.emplace_back(index);
        out.second.emplace_back(reader(data));
    };
    const auto ChooseTxo = [](const auto& outpoint, auto& out) {
        out.first.emplace_back(outpoint);
        out.second.emplace_back(outpoint.Bytes());
    };

    for (const auto& [index, data] : in.elements_20_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseKey(index, data, s20);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_20_;
            const auto& yes = r->second.confirmed_match_.match_20_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s20);
            }
        }
    }

    for (const auto& [index, data] : in.elements_32_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseKey(index, data, s32);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_32_;
            const auto& yes = r->second.confirmed_match_.match_32_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s32);
            }
        }
    }

    for (const auto& [index, data] : in.elements_33_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseKey(index, data, s33);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_33_;
            const auto& yes = r->second.confirmed_match_.match_33_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s33);
            }
        }
    }

    for (const auto& [index, data] : in.elements_64_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseKey(index, data, s64);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_64_;
            const auto& yes = r->second.confirmed_match_.match_64_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s64);
            }
        }
    }

    for (const auto& [index, data] : in.elements_65_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseKey(index, data, s65);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_65_;
            const auto& yes = r->second.confirmed_match_.match_65_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s65);
            }
        }
    }

    for (const auto& [outpoint, output] : in.txos_) {
        if (auto r = in.results_.find(block); in.results_.end() == r) {
            ChooseTxo(outpoint, stxo);
        } else {
            const auto& no = r->second.confirmed_no_match_.match_txo_;
            const auto& yes = r->second.confirmed_match_.match_txo_;

            if ((0u == no.count(outpoint)) && (0u == yes.count(outpoint))) {
                ChooseTxo(outpoint, stxo);
            }
        }
    }
}

auto SubchainStateData::set_key_data(
    block::bitcoin::Transaction& tx) const noexcept -> void
{
    const auto keys = tx.Keys();
    auto data = block::KeyData{};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& key : keys) {
        data.try_emplace(
            key, api.SenderContact(key), api.RecipientContact(key));
    }

    tx.Internal().SetKeyData(data);
}

auto SubchainStateData::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::watchdog_ack: {
            process_watchdog_ack(std::move(msg));
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::prepare_shutdown: {
            transition_state_shutdown();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::update:
        case Work::watchdog:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::init:
        case Work::prepare_shutdown: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::prepare_reorg:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::watchdog_ack: {
            process_watchdog_ack(std::move(msg));
        } break;
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::update:
        case Work::watchdog:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::supported_scripts(const crypto::Element& element)
    const noexcept -> UnallocatedVector<ScriptForm>
{
    auto out = UnallocatedVector<ScriptForm>{};
    using Type = ScriptForm::Type;
    out.emplace_back(api_, element, chain_, Type::PayToPubkey);
    out.emplace_back(api_, element, chain_, Type::PayToPubkeyHash);
    out.emplace_back(api_, element, chain_, Type::PayToWitnessPubkeyHash);

    return out;
}

auto SubchainStateData::to_patterns(const Elements& in, allocator_type alloc)
    const noexcept -> Patterns
{
    auto out = Patterns{alloc};
    const auto subchainID = WalletDatabase::SubchainID{subchain_, id_};
    auto cb = [&](const auto& vector) {
        for (const auto& [index, data] : vector) {
            out.emplace_back(std::make_pair(
                WalletDatabase::ElementID{index, subchainID},
                [&](const auto& source) {
                    auto pattern = Vector<std::byte>{alloc};
                    copy(reader(source), writer(pattern));

                    return pattern;
                }(data)));
        }
    };
    cb(in.elements_20_);
    cb(in.elements_32_);
    cb(in.elements_33_);
    cb(in.elements_64_);
    cb(in.elements_65_);

    return out;
}

auto SubchainStateData::transition_state_normal() noexcept -> void
{
    OT_ASSERT(have_children_);

    disable_automatic_processing_ = false;
    auto rc = scan_->ChangeState(JobState::normal, {});

    OT_ASSERT(rc);

    rc = process_->ChangeState(JobState::normal, {});

    OT_ASSERT(rc);

    rc = index_->ChangeState(JobState::normal, {});

    OT_ASSERT(rc);

    rc = rescan_->ChangeState(JobState::normal, {});

    OT_ASSERT(rc);

    rc = progress_->ChangeState(JobState::normal, {});

    OT_ASSERT(rc);

    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto SubchainStateData::transition_state_reorg(StateSequence id) noexcept
    -> void
{
    OT_ASSERT(0u < id);

    if (0u == reorgs_.count(id)) {
        reorgs_.emplace(id);

        OT_ASSERT(have_children_);

        disable_automatic_processing_ = true;
        auto rc = scan_->ChangeState(JobState::reorg, id);

        OT_ASSERT(rc);

        rc = process_->ChangeState(JobState::reorg, id);

        OT_ASSERT(rc);

        rc = index_->ChangeState(JobState::reorg, id);

        OT_ASSERT(rc);

        rc = rescan_->ChangeState(JobState::reorg, id);

        OT_ASSERT(rc);

        rc = progress_->ChangeState(JobState::reorg, id);

        OT_ASSERT(rc);

        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Flush();
    }
}

auto SubchainStateData::transition_state_shutdown() noexcept -> void
{
    clear_children();
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to shutdown state ").Flush();
    signal_shutdown();
}

auto SubchainStateData::translate(const TXOs& utxos, Patterns& outpoints)
    const noexcept -> void
{
    for (const auto& [outpoint, output] : utxos) {
        OT_ASSERT(output);

        auto keys = output->Keys();

        OT_ASSERT(0 < keys.size());
        // TODO the assertion below will not always be true in the future but
        // for now it will catch some bugs
        OT_ASSERT(1 == keys.size());

        for (auto& key : keys) {
            const auto& [id, subchain, index] = key;
            auto account = api_.Factory().Identifier(id);

            OT_ASSERT(false == account->empty());
            // TODO the assertion below will not always be true in the future
            // but for now it will catch some bugs
            OT_ASSERT(account == id_);

            outpoints.emplace_back(
                WalletDatabase::ElementID{
                    static_cast<Bip32Index>(index),
                    {static_cast<Subchain>(subchain), std::move(account)}},
                space(outpoint.Bytes(), outpoints.get_allocator().resource()));
        }
    }
}

auto SubchainStateData::work() noexcept -> bool
{
    to_children_.Send(MakeWork(Work::watchdog));
    const auto now = Clock::now();
    using namespace std::literals;
    static constexpr auto timeout = 90s;

    for (const auto& [type, time] : child_activity_) {
        const auto interval =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - time);

        if (interval > timeout) {
            LogConsole()(interval)(" elapsed since last activity from ")(
                print(type))(" job for ")(name_)
                .Flush();
        }
    }

    watchdog_.SetRelative(60s);
    watchdog_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(name_)(" ")(error).Flush();
            }
        } else {
            trigger();
        }
    });

    return false;
}

SubchainStateData::~SubchainStateData()
{
    signal_shutdown();
    watchdog_.Cancel();
}
}  // namespace opentxs::blockchain::node::wallet
