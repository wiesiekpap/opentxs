// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <boost/container/container_fwd.hpp>
#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <future>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/BlockOracle.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/BlockOracle.hpp"
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
#include "util/Container.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Thread.hpp"
#include "util/Work.hpp"
#include "util/tuning.hpp"

namespace opentxs
{
// https://baptiste-wicht.com/posts/2014/07/compile-integer-square-roots-at-compile-time-in-cpp.html
static constexpr auto isqrt(std::size_t x, std::size_t r, std::size_t i)
    -> std::size_t
{
    return i == 0 ? r
                  : isqrt(
                        x >= r + i ? x - (r + i) : x,
                        x >= r + i ? (r + 2 * i) >> 1 : r >> 1,
                        i >> 2);
}

static constexpr auto isqrt_i(std::size_t x, std::size_t i) -> std::size_t
{
    return i <= x ? i : isqrt_i(x, i >> 2);
}

static constexpr auto isqrt(std::size_t x) -> std::size_t
{
    return isqrt(x, 0, isqrt_i(x, 1ull << ((sizeof(x) * 8) - 2)));
}
}  // namespace opentxs

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
            {Job::reprocess, "reprocess"},
            {Job::rescan, "rescan"},
            {Job::do_rescan, "do_rescan"},
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
class SubchainStateData::PrehashData
{
public:
    const std::size_t job_count_;

    auto operator()(const std::size_t job) noexcept -> void
    {
        const auto end = targets_.size();

        for (auto i = job; i < end; i += job_count_) {
            hash(targets_.at(i), data_.at(i));
        }
    }

    auto operator()(
        const std::string_view procedure,
        const Log& log,
        const Vector<GCS>& cfilters,
        std::atomic_bool& atLeastOnce,
        const std::size_t job,
        wallet::MatchCache::Results& results,
        MatchResults& data) noexcept -> void
    {
        const auto end = std::min(targets_.size(), cfilters.size());
        auto alloc = results.get_allocator();
        auto cache = std::make_shared<AsyncResults>(std::make_tuple(
            Positions{alloc}, Positions{alloc}, FilterMap{alloc}));

        for (auto i = job; i < end; i += job_count_) {
            atLeastOnce.store(true);
            const auto& cfilter = cfilters.at(i);
            const auto& selected = targets_.at(i);
            const auto& data = data_.at(i);
            const auto position =
                block::Position{std::get<0>(data), selected.first};
            auto& result = results.at(position);
            match(
                procedure,
                log,
                position,
                cfilter,
                selected,
                data,
                *cache,
                result);
        }
        data.modify_detach([cached = std::move(cache)](auto& out) {
            const auto& [iClean, iDirty, iSizes] = *cached;
            auto& [oClean, oDirty, oSizes] = out;
            std::copy(
                iClean.begin(),
                iClean.end(),
                std::inserter(oClean, oClean.end()));
            std::copy(
                iDirty.begin(),
                iDirty.end(),
                std::inserter(oDirty, oDirty.end()));
            std::copy(
                iSizes.begin(),
                iSizes.end(),
                std::inserter(oSizes, oSizes.end()));
        });
    }

    PrehashData(
        const api::Session& api,
        const BlockTargets& targets,
        const std::string_view name,
        wallet::MatchCache::Results& results,
        block::Height start,
        std::size_t jobs,
        allocator_type alloc) noexcept
        : job_count_(jobs)
        , api_(api)
        , targets_(targets)
        , name_(name)
        , data_(alloc)
    {
        OT_ASSERT(0 < job_count_);

        data_.reserve(targets.size());

        for (const auto& [block, elements] : targets) {
            const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
            auto& [height, data20, data32, data33, data64, data65, dataTxo] =
                data_.emplace_back();
            data20.first.reserve(e20.first.size());
            data32.first.reserve(e32.first.size());
            data33.first.reserve(e33.first.size());
            data64.first.reserve(e64.first.size());
            data65.first.reserve(e65.first.size());
            dataTxo.first.reserve(eTxo.first.size());
            height = start++;
            results[block::Position{height, block}];
        }

        OT_ASSERT(targets_.size() == data_.size());
    }

private:
    using Hash = std::uint64_t;
    using Hashes = Vector<Hash>;
    using ElementHashMap = Map<Hash, Vector<const Bip32Index*>>;
    using TxoHashMap = Map<Hash, Vector<const block::Outpoint*>>;
    using ElementData = std::pair<Hashes, ElementHashMap>;
    using TxoData = std::pair<Hashes, TxoHashMap>;
    using BlockData = std::tuple<
        block::Height,
        ElementData,  // 20 byte
        ElementData,  // 32 byte
        ElementData,  // 33 byte
        ElementData,  // 64 byte
        ElementData,  // 65 byte
        TxoData>;
    using Data = Vector<BlockData>;

    const api::Session& api_;
    const BlockTargets& targets_;
    const std::string_view name_;
    Data data_;

    auto hash(const BlockTarget& target, BlockData& row) noexcept -> void
    {
        const auto& [block, elements] = target;
        const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
        auto& [height, data20, data32, data33, data64, data65, dataTxo] = row;
        hash(block, e20, data20);
        hash(block, e32, data32);
        hash(block, e33, data33);
        hash(block, e64, data64);
        hash(block, e65, data65);
        hash(block, eTxo, dataTxo);
    }
    template <typename Input, typename Output>
    auto hash(
        const block::Hash& block,
        const std::pair<Vector<Input>, Targets>& targets,
        Output& dest) noexcept -> void
    {
        const auto key =
            blockchain::internal::BlockHashToFilterKey(block.Bytes());
        const auto& [indices, bytes] = targets;
        auto& [hashes, map] = dest;
        auto i = indices.cbegin();
        auto t = bytes.cbegin();
        auto end = indices.cend();

        for (; i < end; ++i, ++t) {
            auto& hash = hashes.emplace_back(gcs::Siphash(api_, key, *t));
            map[hash].emplace_back(&(*i));
        }

        dedup(hashes);
    }
    auto match(
        const std::string_view procedure,
        const Log& log,
        const block::Position& position,
        const GCS& cfilter,
        const BlockTarget& targets,
        const BlockData& prehashed,
        AsyncResults& cache,
        wallet::MatchCache::Index& results) const noexcept -> void
    {
        const auto alloc = results.get_allocator();
        const auto GetKeys = [&](const auto& data) {
            auto out = Set<Bip32Index>{alloc};
            const auto& [hashes, map] = data;
            const auto start = hashes.cbegin();

            for (const auto& match : cfilter.Internal().Match(hashes)) {
                const auto dist = std::distance(start, match);

                OT_ASSERT(0 <= dist);

                const auto& hash = hashes.at(static_cast<std::size_t>(dist));

                for (const auto* item : map.at(hash)) { out.emplace(*item); }
            }

            return out;
        };
        const auto GetOutpoints = [&](const auto& data) {
            auto out = Set<block::Outpoint>{alloc};
            const auto& [hashes, map] = data;
            const auto start = hashes.cbegin();

            for (const auto& match : cfilter.Internal().Match(hashes)) {
                const auto dist = std::distance(start, match);

                OT_ASSERT(0 <= dist);

                const auto& hash = hashes.at(static_cast<std::size_t>(dist));

                for (const auto* item : map.at(hash)) { out.emplace(*item); }
            }

            return out;
        };
        const auto GetResults = [&](const auto& cb,
                                    const auto& prehashed,
                                    const auto& selected,
                                    auto& clean,
                                    auto& dirty,
                                    auto& output) {
            const auto matches = cb(prehashed);

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
        const auto& [height, p20, p32, p33, p64, p65, pTxo] = prehashed;
        const auto& [s20, s32, s33, s64, s65, sTxo] = selected;
        auto output = std::pair<std::size_t, std::size_t>{};
        GetResults(
            GetKeys,
            p20,
            s20,
            results.confirmed_no_match_.match_20_,
            results.confirmed_match_.match_20_,
            output);
        GetResults(
            GetKeys,
            p32,
            s32,
            results.confirmed_no_match_.match_32_,
            results.confirmed_match_.match_32_,
            output);
        GetResults(
            GetKeys,
            p33,
            s33,
            results.confirmed_no_match_.match_33_,
            results.confirmed_match_.match_33_,
            output);
        GetResults(
            GetKeys,
            p64,
            s64,
            results.confirmed_no_match_.match_64_,
            results.confirmed_match_.match_64_,
            output);
        GetResults(
            GetKeys,
            p65,
            s65,
            results.confirmed_no_match_.match_65_,
            results.confirmed_match_.match_65_,
            output);
        GetResults(
            GetOutpoints,
            pTxo,
            sTxo,
            results.confirmed_no_match_.match_txo_,
            results.confirmed_match_.match_txo_,
            output);
        const auto& [count, of] = output;
        log(OT_PRETTY_CLASS())(name_)(" GCS ")(procedure)(" for block ")(
            position)(" matched ")(count)(" of ")(of)(" target elements")
            .Flush();
        auto& [clean, dirty, sizes] = cache;

        if (0u == count) {
            clean.emplace(position);
        } else {
            dirty.emplace(position);
        }

        sizes.emplace(position.height_, cfilter.ElementCount());
    }
};
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Manager& node,
    database::Wallet& db,
    const node::internal::Mempool& mempool,
    const crypto::Subaccount& subaccount,
    const cfilter::Type filter,
    const crypto::Subchain subchain,
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
          std::string(describe(subaccount, subchain, alloc)),
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
              {SocketType::Push,
               {
                   {CString{node.BlockOracle().Internal().Endpoint(), alloc},
                    Direction::Connect},
               }},
              {SocketType::Publish,
               {
                   {toChildren, Direction::Bind},
               }},
              {SocketType::Push,
               {
                   {toScan, Direction::Connect},
               }},
          })
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_oracle_(mempool)
    , subaccount_(subaccount)
    , owner_(subaccount_.Parent().NymID())
    , account_type_(subaccount_.Type())
    , id_(subaccount_.ID())
    , subchain_(subchain)
    , chain_(node_.Chain())
    , filter_type_(filter)
    , db_key_(db.GetSubchainID(id_, subchain_))
    , null_position_(block::Position{})
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
    , from_parent_(parent, alloc)
    , scan_threshold_(1000)
    , maximum_scan_(2000)
    , element_cache_(
          db_.GetPatterns(db_key_, alloc.resource()),
          db_.GetUnspentOutputs(id_, subchain_, alloc.resource()),
          alloc)
    , match_cache_(alloc)
    , scan_dirty_(false)
    , process_queue_(0)
    , progress_position_(std::nullopt)
    , to_block_oracle_(pipeline_.Internal().ExtraSocket(0))
    , to_children_(pipeline_.Internal().ExtraSocket(1))
    , to_scan_(pipeline_.Internal().ExtraSocket(2))
    , pending_state_(State::normal)
    , state_(State::normal)
    , filter_sizes_(alloc)
    , elements_per_cfilter_(0)
    , job_counter_()
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
    tdiag("SubchainStateData::SubchainStateData.1", this);
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Manager& node,
    database::Wallet& db,
    const node::internal::Mempool& mempool,
    const crypto::Subaccount& subaccount,
    const cfilter::Type filter,
    const crypto::Subchain subchain,
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
    tdiag("SubchainStateData::SubchainStateData.2", this);
}

auto SubchainStateData::ChangeState(
    const State state,
    StateSequence reorg) noexcept -> bool
{
    return synchronize(
        [state, reorg, this] { return sChangeState(state, reorg); });
}
auto SubchainStateData::sChangeState(
    const State state,
    StateSequence reorg) noexcept -> bool
{
    if (auto old = pending_state_.exchange(state); old == state) {
        return true;
    }

    tdiag("sChangeState ", this);
    auto output{false};

    switch (state) {
        case State::normal: {
            if (State::reorg != state_) { break; }

            output = transition_state_normal();
        } break;
        case State::reorg: {
            if (State::shutdown == state_) { break; }

            output = transition_state_reorg(reorg);
        } break;
        case State::shutdown: {
            if (State::reorg == state_) { break; }

            output = transition_state_shutdown();
        } break;
        default: {
            OT_FAIL;
        }
    }

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())(name_)(" failed to change state from ")(
            print(state_))(" to ")(print(state))
            .Flush();
    }

    return output;
}

auto SubchainStateData::choose_thread_count(std::size_t elements) noexcept
    -> std::size_t
{
    // NOTE the target thread count is the square root of the number of
    // elements divided by 512. The minimum value is one and the maximum
    // value is one less than the number of hardware threads.
    static constexpr auto calc = [](auto elements, auto hardware) {
        const auto limit = std::max<std::size_t>(
            static_cast<std::size_t>(isqrt(elements >> 9u)), 1u);

        return std::min<std::size_t>(
            limit, std::max<std::size_t>(hardware, 2u) - 1u);
    };

    static_assert(isqrt(0) == 0);
    static_assert(isqrt(1) == 1);
    static_assert(isqrt(3) == 1);
    static_assert(isqrt(4) == 2);
    static_assert(isqrt(5) == 2);
    static_assert(isqrt(8) == 2);
    static_assert(isqrt(9) == 3);
    static_assert(calc(0, 100) == 1);
    static_assert(calc(2047, 100) == 1);
    static_assert(calc(2048, 100) == 2);
    static_assert(calc(4608, 100) == 3);
    static_assert(calc(8192, 100) == 4);
    static_assert(calc(8192, 5) == 4);
    static_assert(calc(8192, 4) == 3);
    static_assert(calc(8192, 3) == 2);
    static_assert(calc(8192, 2) == 1);
    static_assert(calc(8192, 1) == 1);
    static_assert(calc(8192, 0) == 1);
    static_assert(calc(50000, 100) == 9);
    static_assert(calc(51200, 100) == 10);

    return calc(elements, std::thread::hardware_concurrency());
}

auto SubchainStateData::clear_children() noexcept -> void
{
    tdiag("clear_children, children:", (have_children_ ? "YES" : "NO"));
    if (have_children_) {
        have_children_ = false;
        tdiag("SubchainStateData clear_children.1 ", this);

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

        tdiag("SubchainStateData clear_children.2 ", this);

        scan_.reset();
        process_.reset();
        index_.reset();
        rescan_.reset();
        progress_.reset();
    }
}

auto SubchainStateData::describe(
    const crypto::Subaccount& account,
    const crypto::Subchain subchain,
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
    const block::Position& ancestor) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" processing reorg to ")(ancestor).Flush();
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
            scan_->ProcessReorg(headerOracleLock, ancestor);
            process_->ProcessReorg(headerOracleLock, ancestor);
            index_->ProcessReorg(headerOracleLock, ancestor);
            rescan_->ProcessReorg(headerOracleLock, ancestor);
            progress_->ProcessReorg(headerOracleLock, ancestor);
        } else {

            ++errors;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            name_)(" header oracle claims existing tip ")(tip)(" is invalid")
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

auto SubchainStateData::highest_clean(
    const AsyncResults& results,
    block::Position& highestTested) noexcept -> std::optional<block::Position>
{
    const auto& [clean, dirty, sizes] = results;
    const auto haveClean = (0 < clean.size());
    const auto haveDirty = (0 < dirty.size());

    if ((false == haveClean) && haveDirty) {
        highestTested = *dirty.crbegin();

        return std::nullopt;
    } else if ((false == haveDirty) && haveClean) {
        highestTested = *clean.crbegin();

        return highestTested;
    } else if (haveClean && haveDirty) {
        highestTested = std::max(*clean.crbegin(), *dirty.crbegin());
        const auto& lowestDirty = *dirty.cbegin();

        if (auto i = clean.upper_bound(lowestDirty); clean.begin() == i) {

            return std::nullopt;
        } else {

            return *std::prev(i);
        }
    } else {

        return std::nullopt;
    }
}

auto SubchainStateData::IndexElement(
    const cfilter::Type type,
    const blockchain::crypto::Element& input,
    const Bip32Index index,
    database::Wallet::ElementMap& output) const noexcept -> void
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

auto SubchainStateData::to_str(Work w) const noexcept -> std::string
{
    return std::string(print(w));
}

auto SubchainStateData::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    tadiag("pipeline ", std::string{print(work)});

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

auto SubchainStateData::process_rescan(Message&& in) noexcept -> void
{
    to_scan_.Send(MakeWork(Work::do_rescan));
}

auto SubchainStateData::process_watchdog_ack(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    child_activity_.at(body.at(1).as<JobType>()) = Clock::now();
}

auto SubchainStateData::ProcessBlock(
    const block::Position& position,
    const bitcoin::block::Block& block) const noexcept -> bool
{
    const auto start = Clock::now();
    const auto& name = name_;
    const auto& type = filter_type_;
    const auto& node = node_;
    const auto& filters = node.FilterOracleInternal();
    const auto& blockHash = position.hash_;
    auto buf = std::array<std::byte, 16_KiB>{};
    auto upstream = alloc::StandardToBoost{get_allocator().resource()};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size(), &upstream};
    auto haveTargets = Time{};
    auto haveFilter = Time{};
    auto keyMatches = 0_uz;
    auto txoMatches = 0_uz;
    const auto& log = LogTrace();
    // Try to remove Lambda
    const auto confirmed = [&] {
        const auto handle = element_cache_.lock_shared();
        const auto matches = match_cache_.lock_shared()->GetMatches(position);
        const auto& elements = handle->GetElements();
        auto patterns = std::make_pair(Patterns{&alloc}, Patterns{&alloc});

        if (false == select_matches(matches, position, elements, patterns)) {
            // TODO blocks should only be queued for processing if they have
            // been previously marked by a scan or rescan operation which
            // updates the cache with the appropriate entries so it's not clear
            // why this branch can ever be reached.
            select_all(position, elements, patterns);
        }

        haveTargets = Clock::now();
        const auto cfilter =
            filters.LoadFilter(type, blockHash, get_allocator());

        OT_ASSERT(cfilter.IsValid());

        haveFilter = Clock::now();
        const auto& [outpoint, key] = patterns;
        keyMatches = key.size();
        txoMatches = outpoint.size();

        return block.Internal().FindMatches(type, outpoint, key, log);
    }();
    const auto haveMatches = Clock::now();
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    const auto haveHeader = Clock::now();
    handle_confirmed_matches(block, position, confirmed, log);
    const auto handledMatches = Clock::now();
    LogConsole()(name)(" processed block ")(position)(" in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(general.size())(" of ")(
        keyMatches)(" potential key matches confirmed.")
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(utxo.size())(" of ")(
        txoMatches)(" potential utxo matches confirmed.")
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

    return true;
}

auto SubchainStateData::ProcessTransaction(
    const bitcoin::block::Transaction& tx,
    const Log& log) const noexcept -> void
{
    auto buf = std::array<std::byte, 4_KiB>{};
    auto upstream = alloc::StandardToBoost{get_allocator().resource()};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size(), &upstream};
    auto copy = tx.clone();

    OT_ASSERT(copy);

    blockchain::block::Matches matches{};
    {
        auto handle = element_cache_.lock_shared();
        const auto& elements = handle->GetElements();
        const auto targets = get_account_targets(elements, &alloc);
        const auto patterns = to_patterns(elements, &alloc);
        const auto parsed = block::ParsedPatterns{patterns};
        auto outpoints = SubchainStateData::Patterns{&alloc};
        translate(elements.txos_, outpoints);

        matches =
            copy->Internal().FindMatches(filter_type_, outpoints, parsed, log);
    }
    handle_mempool_matches(matches, std::move(copy));
}

auto SubchainStateData::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> void
{
    synchronize(
        [&lock = std::as_const(headerOracleLock), &tx, &errors, &parent, this] {
            sProcessReorg(lock, tx, errors, parent);
        });
}

auto SubchainStateData::sProcessReorg(
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
    log_(OT_PRETTY_CLASS())(name_)(" progress updated to ")(pos).Flush();
    subaccount_.Internal().SetScanProgress(pos, subchain_);
    api_.Crypto().Blockchain().Internal().ReportScan(
        chain_, owner_, account_type_, id_, subchain_, pos);
}

auto SubchainStateData::reorg_children() const noexcept -> std::size_t
{
    return 1u;
}

auto SubchainStateData::ReorgTarget(
    const Lock& headerOracleLock,
    const block::Position& reorg,
    const block::Position& current) noexcept -> block::Position
{
    return std::min(current, reorg);
}

auto SubchainStateData::Rescan(
    const block::Position& best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out) const noexcept -> std::optional<block::Position>
{
    return scan(true, best, stop, highestTested, out);
}

auto SubchainStateData::Scan(
    const block::Position& best,
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
    try {
        using namespace std::literals;
        const auto procedure = rescan ? "rescan"sv : "scan"sv;
        const auto& log = log_;
        const auto& name = name_;
        const auto& node = node_;
        const auto& type = filter_type_;
        const auto& headers = node.HeaderOracle();
        const auto& filters = node.FilterOracleInternal();
        const auto start = Clock::now();
        const auto startHeight = highestTested.height_ + 1;
        auto atLeastOnce = std::atomic_bool{false};
        auto highestClean = std::optional<block::Position>{std::nullopt};
        auto resultMap = [&] {
            std::size_t elementsPerFilter;
            const auto cached = elements_per_cfilter_.load();
            if (0u == cached) {
                const auto chainDefault =
                    params::Chains().at(chain_).cfilter_element_count_estimate_;
                elementsPerFilter = std::max<std::size_t>(1u, chainDefault);
            } else {
                elementsPerFilter = cached;
            }

            OT_ASSERT(0u < elementsPerFilter);

            constexpr auto GetBatchSize = [](std::size_t cfilter,
                                             std::size_t user) {
                constexpr auto cfilterWeight = 1_uz;
                constexpr auto walletWeight = 5_uz;
                constexpr auto target = 425000_uz;
                constexpr auto max = 10000_uz;

                return std::min<std::size_t>(
                    std::max<std::size_t>(
                        (target * (cfilterWeight * walletWeight)) /
                            ((cfilterWeight * cfilter) + (walletWeight * user)),
                        1u),
                    max);
            };
            static_assert(GetBatchSize(1, 1) == 10000);
            static_assert(GetBatchSize(25, 40) == 9444);
            static_assert(GetBatchSize(1000, 40) == 1770);
            static_assert(GetBatchSize(25, 400) == 1049);
            static_assert(GetBatchSize(1000, 400) == 708);
            static_assert(GetBatchSize(25, 4000) == 106);
            static_assert(GetBatchSize(25, 40000) == 10);
            static_assert(GetBatchSize(1000, 40000) == 10);
            static_assert(GetBatchSize(25, 400000) == 1);
            static_assert(GetBatchSize(25, 4000000) == 1);
            static_assert(GetBatchSize(10000, 4000000) == 1);
            auto handle = element_cache_.lock_shared();
            const auto& elements = handle->GetElements();
            const auto elementCount =
                std::max<std::size_t>(elements.size(), 1u);
            // NOTE attempting to scan too many filters at once causes this
            // function to take excessive time to execute, which means the Scan
            // and Rescan Actors will be unable to process new messages for an
            // extended amount of time which has many negative side effects. The
            // GetBatchSize function attempts to prevent this from happening by
            // limiting the batch size to a reasonable value based on the
            // average cfilter element count (estimated) and match set for this
            // subchain (known).
            const auto threads = choose_thread_count(elementCount);
            const auto scanBatch = std::min(
                maximum_scan_,
                GetBatchSize(elementsPerFilter, elementCount) * threads);
            log(OT_PRETTY_CLASS())(name)(" filter size: ")(
                elementsPerFilter)(" wallet size: ")(
                elementCount)(" batch size: ")(scanBatch)
                .Flush();
            const auto stopHeight = std::min(
                std::min<block::Height>(
                    startHeight + scanBatch - 1, best.height_),
                stop);

            if (startHeight > stopHeight) {
                log(OT_PRETTY_CLASS())(name)(" attempted to ")(
                    procedure)(" filters from ")(startHeight)(" to ")(
                    stopHeight)(" but this is impossible")
                    .Flush();

                throw std::runtime_error{""};
            }

            log(OT_PRETTY_CLASS())(name)(" ")(procedure)("ning filters from ")(
                startHeight)(" to ")(stopHeight)
                .Flush();
            const auto target =
                static_cast<std::size_t>(stopHeight - startHeight + 1);
            const auto blocks = headers.BestHashes(
                startHeight, target, get_allocator().resource());
            auto filterPromise = std::promise<Vector<GCS>>{};
            auto filterFuture = filterPromise.get_future();
            auto tp = api_.Network().Asio().Internal().Post(
                ThreadPool::General,
                [&] {
                    filterPromise.set_value(filters.LoadFilters(type, blocks));
                },
                subchainStateDataFilterThreadName);

            if (false == tp) { throw std::runtime_error{""}; }

            auto selected = BlockTargets{get_allocator()};
            select_targets(*handle, blocks, elements, startHeight, selected);
            auto results = wallet::MatchCache::Results{get_allocator()};
            auto prehash = PrehashData{
                api_,
                selected,
                name_,
                results,
                startHeight,
                std::min(threads, selected.size()),
                get_allocator()};

            if (1u < prehash.job_count_) {
                auto count = job_counter_.Allocate();

                for (auto n{0u}; n < prehash.job_count_; ++n) {
                    tp = api_.Network().Asio().Internal().Post(
                        ThreadPool::General,
                        [post = std::make_shared<ScopeGuard>(
                             [&count] { ++count; }, [&] { --count; }),
                         n,
                         &prehash] { prehash(n); },
                        subchainStateDataPrehash1ThreadName);

                    if (false == tp) { throw std::runtime_error{""}; }
                }
            } else {
                prehash(0u);
            }

            const auto havePrehash = Clock::now();
            log_(OT_PRETTY_CLASS())(name)(" ")(
                procedure)(" calculated target hashes for ")(blocks.size())(
                " cfilters in ")(std::chrono::nanoseconds{havePrehash - start})
                .Flush();

            auto cfilters = filterFuture.get();

            cfilters.erase(
                std::find_if(
                    cfilters.begin(),
                    cfilters.end(),
                    [](const auto& filter) {
                        return false == filter.IsValid();
                    }),
                cfilters.end());

            const auto haveCfilters = Clock::now();
            log_(OT_PRETTY_CLASS())(name)(" ")(
                procedure)(" loaded cfilters in ")(
                std::chrono::nanoseconds{haveCfilters - havePrehash})
                .Flush();
            const auto cfilterCount = cfilters.size();

            OT_ASSERT(cfilterCount <= blocks.size());

            auto data = MatchResults{std::make_tuple(
                Positions{get_allocator()},
                Positions{get_allocator()},
                FilterMap{get_allocator()})};

            OT_ASSERT(0u < selected.size());

            if ((1u < prehash.job_count_)) {
                auto count = job_counter_.Allocate();

                for (auto n{0u}; n < prehash.job_count_; ++n) {
                    tp = api_.Network().Asio().Internal().Post(
                        ThreadPool::General,
                        [&,
                         n,
                         post = std::make_shared<ScopeGuard>(
                             [&count] { ++count; }, [&] { --count; })] {
                            prehash(
                                procedure,
                                log,
                                cfilters,
                                atLeastOnce,
                                n,
                                results,
                                data);
                        },
                        subchainStateDataPrehash2ThreadName);

                    if (false == tp) { throw std::runtime_error{""}; }
                }
            } else {
                prehash(
                    procedure, log, cfilters, atLeastOnce, 0u, results, data);
            }

            {
                auto handle = data.lock_shared();
                const auto& [clean, dirty, sizes] = *handle;

                if (auto size = dirty.size(); 0 < size) {
                    log_(OT_PRETTY_CLASS())(name_)(" requesting ")(
                        size)(" block hashes from block oracle")
                        .Flush();
                    auto work = MakeWork(node::BlockOracleJobs::request_blocks);

                    for (const auto& position : dirty) {
                        const auto& [height, hash] = position;
                        work.AddFrame(hash);
                        out.emplace_back(ScanState::dirty, position);
                    }
                    to_block_oracle_.SendDeferred(std::move(work));
                }

                highestClean = highest_clean(*handle, highestTested);

                if (false == rescan) {
                    std::transform(
                        sizes.begin(),
                        sizes.end(),
                        std::back_inserter(filter_sizes_),
                        [](const auto& in) { return in.second; });

                    // NOTE these statements calculate a 1000 block (or whatever
                    // cfilter_size_window_ is set to) simple moving average of
                    // cfilter element sizes

                    while (cfilter_size_window_ < filter_sizes_.size()) {
                        filter_sizes_.pop_front();
                    }

                    const auto totalCfilterElements = std::accumulate(
                        filter_sizes_.begin(), filter_sizes_.end(), 0_uz);
                    elements_per_cfilter_.store(std::max(
                        1_uz, totalCfilterElements / filter_sizes_.size()));
                }
            }

            return results;
        }();

        if (atLeastOnce.load()) {
            if (0u < resultMap.size()) {
                match_cache_.lock()->Add(std::move(resultMap));
            }

            const auto count = out.size();
            log(OT_PRETTY_CLASS())(name)(" ")(procedure)(" found ")(
                count)(" new potential matches between blocks ")(
                startHeight)(" and ")(highestTested.height_)(" in ")(
                std::chrono::nanoseconds{Clock::now() - start})
                .Flush();
        } else {
            log_(OT_PRETTY_CLASS())(name)(" ")(procedure)(" interrupted")
                .Flush();
        }

        return highestClean;
    } catch (...) {

        return std::nullopt;
    }
}

auto SubchainStateData::select_all(
    const block::Position& block,
    const Elements& in,
    MatchesToTest& out) const noexcept -> void
{
    const auto subchainID = database::Wallet::SubchainID{subchain_, id_};
    auto& [outpoint, key] = out;
    auto alloc = outpoint.get_allocator();
    const auto SelectKey = [&](const auto& all, auto& out) {
        for (const auto& [index, data] : all) {
            out.emplace_back(std::make_pair(
                std::make_pair(index, subchainID),
                space(reader(data), alloc.resource())));
        }
    };
    const auto SelectTxo = [&](const auto& all, auto& out) {
        const auto self = id_->str();

        for (const auto& [outpoint, pOutput] : all) {
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
    };

    SelectKey(in.elements_20_, key);
    SelectKey(in.elements_32_, key);
    SelectKey(in.elements_33_, key);
    SelectKey(in.elements_64_, key);
    SelectKey(in.elements_65_, key);
    SelectTxo(in.txos_, outpoint);
}

auto SubchainStateData::select_matches(
    const std::optional<wallet::MatchCache::Index>& matches,
    const block::Position& block,
    const Elements& in,
    MatchesToTest& out) const noexcept -> bool
{
    const auto subchainID = database::Wallet::SubchainID{subchain_, id_};
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

    if (matches.has_value()) {
        const auto& items = matches->confirmed_match_;
        SelectKey(in.elements_20_, items.match_20_, key);
        SelectKey(in.elements_32_, items.match_32_, key);
        SelectKey(in.elements_33_, items.match_33_, key);
        SelectKey(in.elements_64_, items.match_64_, key);
        SelectKey(in.elements_65_, items.match_65_, key);
        SelectTxo(in.txos_, items.match_txo_, outpoint);

        return true;
    } else {
        // TODO this should never happen because a block is only processed when
        // a previous call to SubchainStateData::scan indicated this block had a
        // match, and the scan function should have set the matched elements in
        // the element cache. The only time this data is deleted is the call to
        // ElementCache::Forget, but that should never be called until there is
        // no possibility that blocks at or below that position will be
        // processed.
        log_(OT_PRETTY_CLASS())(name_)(" existing matches for block ")(
            block)(" not found")
            .Flush();

        return false;
    }
}

auto SubchainStateData::select_targets(
    const wallet::ElementCache& cache,
    const BlockHashes& hashes,
    const Elements& in,
    block::Height height,
    BlockTargets& out) const noexcept -> void
{
    for (const auto& hash : hashes) {
        select_targets(cache, block::Position{height++, hash}, in, out);
    }
}

auto SubchainStateData::select_targets(
    const wallet::ElementCache& cache,
    const block::Position& block,
    const Elements& in,
    BlockTargets& out) const noexcept -> void
{
    auto alloc = out.get_allocator();
    auto& [hash, selected] = out.emplace_back(std::make_pair(
        block.hash_,
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
    const auto matches = match_cache_.lock_shared()->GetMatches(block);

    for (const auto& [index, data] : in.elements_20_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_20_;
            const auto& yes = matches->confirmed_match_.match_20_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s20);
            }
        } else {
            ChooseKey(index, data, s20);
        }
    }

    for (const auto& [index, data] : in.elements_32_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_32_;
            const auto& yes = matches->confirmed_match_.match_32_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s32);
            }
        } else {
            ChooseKey(index, data, s32);
        }
    }

    for (const auto& [index, data] : in.elements_33_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_33_;
            const auto& yes = matches->confirmed_match_.match_33_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s33);
            }
        } else {
            ChooseKey(index, data, s33);
        }
    }

    for (const auto& [index, data] : in.elements_64_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_64_;
            const auto& yes = matches->confirmed_match_.match_64_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s64);
            }
        } else {
            ChooseKey(index, data, s64);
        }
    }

    for (const auto& [index, data] : in.elements_65_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_65_;
            const auto& yes = matches->confirmed_match_.match_65_;

            if ((0u == no.count(index)) && (0u == yes.count(index))) {
                ChooseKey(index, data, s65);
            }
        } else {
            ChooseKey(index, data, s65);
        }
    }

    for (const auto& [outpoint, output] : in.txos_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_txo_;
            const auto& yes = matches->confirmed_match_.match_txo_;

            if ((0u == no.count(outpoint)) && (0u == yes.count(outpoint))) {
                ChooseTxo(outpoint, stxo);
            }
        } else {
            ChooseTxo(outpoint, stxo);
        }
    }
}

auto SubchainStateData::set_key_data(
    bitcoin::block::Transaction& tx) const noexcept -> void
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
        case Work::rescan: {
            process_rescan(std::move(msg));
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
        case Work::reprocess:
        case Work::do_rescan:
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
        case Work::rescan:
        case Work::statemachine: {
            tdiag("...defer(&&)");
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
        case Work::reprocess:
        case Work::do_rescan:
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
    const auto subchainID = database::Wallet::SubchainID{subchain_, id_};
    auto cb = [&](const auto& vector) {
        for (const auto& [index, data] : vector) {

            auto pattern = Vector<std::byte>{alloc};
            copy(reader(data), writer(pattern));

            out.emplace_back(std::make_pair(
                database::Wallet::ElementID{index, subchainID}, pattern));
        }
    };
    cb(in.elements_20_);
    cb(in.elements_32_);
    cb(in.elements_33_);
    cb(in.elements_64_);
    cb(in.elements_65_);

    return out;
}

auto SubchainStateData::transition_state_normal() noexcept -> bool
{
    tdiag(
        "transition_state_normal, children:", (have_children_ ? "YES" : "NO"));
    if (!have_children_) { return false; }

    disable_automatic_processing(false);
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

    return true;
}

auto SubchainStateData::transition_state_reorg(StateSequence id) noexcept
    -> bool
{
    tdiag("transition_state_reorg, children:", (have_children_ ? "YES" : "NO"));
    if (!have_children_) { return false; }

    OT_ASSERT(0u < id);

    auto output{true};

    if (0u == reorgs_.count(id)) {
        output &= scan_->ChangeState(JobState::reorg, id);
        output &= process_->ChangeState(JobState::reorg, id);
        output &= index_->ChangeState(JobState::reorg, id);
        output &= rescan_->ChangeState(JobState::reorg, id);
        output &= progress_->ChangeState(JobState::reorg, id);

        if (false == output) { return false; }

        reorgs_.emplace(id);
        disable_automatic_processing(true);
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Flush();
    }

    return output;
}

auto SubchainStateData::transition_state_shutdown() noexcept -> bool
{
    tdiag(
        "transition_state_shutdown, children:",
        (have_children_ ? "YES" : "NO"));
    clear_children();
    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to shutdown state ").Flush();
    signal_shutdown();

    return true;
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
                database::Wallet::ElementID{
                    static_cast<Bip32Index>(index),
                    {static_cast<crypto::Subchain>(subchain),
                     std::move(account)}},
                space(outpoint.Bytes(), outpoints.get_allocator().resource()));
        }
    }
}

auto SubchainStateData::work() noexcept -> int
{
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

    // trigger in a while
    return SM_SubchainStateData_slow;
}

SubchainStateData::~SubchainStateData()
{
    signal_shutdown();
    watchdog_.Cancel();
}
}  // namespace opentxs::blockchain::node::wallet
