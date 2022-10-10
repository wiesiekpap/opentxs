// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated

#include <opentxs/util/Log.hpp>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>

#include "Proto.tpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/core/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "serialization/protobuf/PaymentEvent.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "util/Container.hpp"
#include "util/Thread.hpp"
#include "util/tuning.hpp"

namespace opentxs::factory
{
auto BlockchainAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>
{
    using ReturnType = ui::implementation::BlockchainAccountActivity;
    const auto [chain, owner] =
        api.Crypto().Blockchain().LookupAccount(accountID);

    OT_ASSERT(owner == nymID);

    return std::make_unique<ReturnType>(api, chain, nymID, accountID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountActivity::BlockchainAccountActivity(
    const api::session::Client& api,
    const blockchain::Type chain,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback& cb) noexcept
    : AccountActivity(api, nymID, accountID, AccountType::Blockchain, cb)
    , chain_(chain)
    , confirmed_(0)
    , balance_cb_(network::zeromq::ListenCallback::Factory(
          [this](auto&& in) { pipeline_.Push(std::move(in)); }))
    , balance_socket_(Widget::api_.Network().ZeroMQ().DealerSocket(
          balance_cb_,
          network::zeromq::socket::Direction::Connect,
          blockchainAccountActivityThreadName))
    , progress_()
    , height_(0)
    , last_job_{}
    , transactions_{}
    , active_{}
    , iload_{transactions_.cend()}
    , eload_{transactions_.cend()}
    , load_in_progress_{}
    , load_lock_{}
    , active_lock_{}
{
    const auto connected = balance_socket_->Start(
        Widget::api_.Endpoints().BlockchainBalance().data());

    OT_ASSERT(connected);

    start();
    init({
        UnallocatedCString{api.Endpoints().BlockchainReorg()},
        UnallocatedCString{api.Endpoints().BlockchainStateChange()},
        UnallocatedCString{api.Endpoints().BlockchainSyncProgress()},
        UnallocatedCString{api.Endpoints().BlockchainTransactions()},
        UnallocatedCString{api.Endpoints().BlockchainTransactions(nymID)},
        UnallocatedCString{api.Endpoints().ContactUpdate()},
    });
    balance_socket_->Send([&] {
        using Job = api::crypto::blockchain::BalanceOracleJobs;
        auto work = network::zeromq::tagged_message(Job::registration);
        work.AddFrame(chain_);
        work.AddFrame(nymID);

        return work;
    }());
}

auto BlockchainAccountActivity::DepositAddress(
    const blockchain::Type chain) const noexcept -> UnallocatedCString
{
    if ((blockchain::Type::Unknown != chain) && (chain_ != chain)) {
        return {};
    }

    const auto& wallet =
        Widget::api_.Crypto().Blockchain().Account(primary_id_, chain_);
    const auto reason = Widget::api_.Factory().PasswordPrompt(
        "Calculating next deposit address");
    const auto& styles = blockchain::params::Chains().at(chain_).styles_;

    if (0u == styles.size()) { return {}; }

    return wallet.GetDepositAddress(styles.front().first, reason);
}

auto BlockchainAccountActivity::display_balance(
    opentxs::Amount value) const noexcept -> UnallocatedCString
{
    return blockchain::internal::Format(chain_, value);
}

enum class Diag {
    load_thread,
    portion_p,
    delete_inactive,
    emplace,
    load_transaction_bitcoin,
    process_txid,
    blockchain_thread_item_id,
    process_txid_lambda,
    process_txid_custom,
    add_item,
    end
};

std::string to_string(Diag d);

std::string to_string(Diag d)
{
    switch (d) {
        case Diag::load_thread:
            return "load_thread    ";
        case Diag::portion_p:
            return "portion_p    ";
        case Diag::delete_inactive:
            return "delete_inactive";
        case Diag::emplace:
            return "emplace        ";
        case Diag::load_transaction_bitcoin:
            return "lo.._bitcoin   ";
        case Diag::process_txid:
            return "process_txid   ";
        case Diag::blockchain_thread_item_id:
            return "bl.._item_id   ";
        case Diag::process_txid_lambda:
            return "pr.._lambda    ";
        case Diag::process_txid_custom:
            return "pr.._custom    ";
        case Diag::add_item:
            return "add_item       ";
        case Diag::end:
            return "end";
        default:
            return "???";
    }
}

auto BlockchainAccountActivity::load_thread() noexcept -> void
{
    bool go{};
    {
        std::unique_lock<std::mutex> L(load_lock_);
        if (load_in_progress_) {
            tdiag("ZZZZ load_thread load_in_progress_");
            return;
        }
        tdiag("ZZZZ load_thread");
        load_in_progress_ = true;
        transactions_ = [&]() -> UnallocatedVector<blockchain::block::pTxid> {
            try {
                auto& chain =
                    Widget::api_.Network().Blockchain().GetChain(chain_);
                height_ = chain.HeaderOracle().BestChain().height_;

                return chain.Internal().GetTransactions(primary_id_);
            } catch (...) {
                tdiag("ZZZZ load_thread transactions exception");
                return {};
            }
        }();
        iload_ = transactions_.cbegin();
        eload_ = transactions_.cend();
        active_.clear();
        go = iload_ != eload_;
    }

    if (go) {
        tdiag("ZZZZ load_thread start, n=", transactions_.size());
        PTimer<Diag>::start(Diag::load_thread);
        trigger();
    } else {
        load_in_progress_ = false;
    }
}

auto BlockchainAccountActivity::load_thread_portion() noexcept -> bool
{
    using namespace std::chrono;
    static thread_local auto cc = 0ull;
    int ct = 0;
    std::unique_lock<std::mutex> L(load_lock_);
    auto Tstart = Clock::now();
    while (iload_ != eload_) {
        auto txid = *iload_;
        ++iload_;
        ++cc;
        if (const auto id = process_txid(txid); id.has_value()) {
            PTimer<Diag>::start(Diag::emplace);
            active_.emplace(id.value());
            PTimer<Diag>::stop(Diag::emplace);
        }
        using namespace std::literals;
        if (++ct == 50) {
            ct = 0;
            if (Clock::now() - Tstart > 200ms) { break; }
        }
    }
    if (iload_ == eload_) {
        cc = 0;
        load_in_progress_ = false;
        transactions_.clear();
        iload_ = transactions_.cbegin();
        eload_ = transactions_.cend();
        PTimer<Diag>::start(Diag::delete_inactive);
        delete_inactive(active_);
        PTimer<Diag>::stop(Diag::delete_inactive);
        PTimer<Diag>::stop(Diag::load_thread);
        for (auto i = Diag::load_thread; i != Diag::end;
             i = static_cast<Diag>(static_cast<int>(i) + 1)) {
            auto t = PTimer<Diag>::gett(i);
            switch (t.ct_out_) {
                case 0:
                    break;
                case 1:
                    tdiag(to_string(i) + " avg: " + std::to_string(t.avg_us()));
                    break;
                default:
                    tdiag(
                        to_string(i) + " avg: " + std::to_string(t.avg_us()) +
                        "us, min: " + std::to_string(t.min_us_) +
                        "us, max: " + std::to_string(t.max_us_));
            }
        }
        PTimer<Diag>::clear();
    }
    return load_in_progress_;
}

auto BlockchainAccountActivity::load_thread_portion_p() noexcept -> bool
{
    using namespace std::chrono;
    std::unique_lock<std::mutex> L(load_lock_);

    constexpr const unsigned threadcount = 4;
    constexpr const unsigned stride = 50;

    PTimer<Diag>::start(Diag::portion_p);
    tdiag("PPP 1");
    std::vector<std::future<unsigned>> fvec = [=]() {
        std::vector<std::future<unsigned>> v{};
        for (unsigned i = 0; i != threadcount; ++i) {
            auto ib = iload_ + i * stride;
            auto ie = (eload_ - ib) > stride ? ib + stride : eload_;
            v.emplace_back(
                std::async(std::launch::async, [ib, ie, this]() -> unsigned {
                    return load_portion(ib, ie);
                }));
            if (ie == eload_) { break; }
        }
        return v;
    }();
    tdiag("PPP 2");
    for (auto& f : fvec) {
        try {
            unsigned step = f.get();
            iload_ += step;
        } catch (const std::exception& e) {
            tdiag("load_portion exception", e.what());
        }
    }
    PTimer<Diag>::stop(Diag::portion_p);
    tdiag("PPP 3, e-b=", (eload_ - iload_));
    if (iload_ == eload_) {
        load_in_progress_ = false;
        transactions_.clear();
        iload_ = transactions_.cbegin();
        eload_ = transactions_.cend();
        PTimer<Diag>::start(Diag::delete_inactive);
        delete_inactive(active_);
        PTimer<Diag>::stop(Diag::delete_inactive);
        PTimer<Diag>::stop(Diag::load_thread);
        for (auto i = Diag::load_thread; i != Diag::end;
             i = static_cast<Diag>(static_cast<int>(i) + 1)) {
            auto t = PTimer<Diag>::gett(i);
            if (t.ct_out_ > 1) {
                tdiag(
                    to_string(i) + " avg: " + std::to_string(t.avg_us()) +
                    "us, min: " + std::to_string(t.min_us_) +
                    "us, max: " + std::to_string(t.max_us_));
            } else if (t.ct_out_ == 1) {
                tdiag(to_string(i) + " avg: " + std::to_string(t.avg_us()));
            }
        }
        PTimer<Diag>::clear();
    }
    return load_in_progress_;
}
auto BlockchainAccountActivity::load_portion(
    UnallocatedVector<blockchain::block::pTxid>::const_iterator ibeg,
    UnallocatedVector<blockchain::block::pTxid>::const_iterator iend) noexcept
    -> unsigned
{
    //    std::cerr << "PPP X k=" << (iend - ibeg) << "\n";
    auto i0 = ibeg;
    while (ibeg != iend) {
        auto txid = *ibeg;
        ++ibeg;
        if (const auto id = process_txid(txid); id.has_value()) {
            std::unique_lock<std::mutex> L(active_lock_);
            active_.emplace(id.value());
        }
    }
    //    std::cerr << "PPP Y r=" << (ibeg - i0) << "\n";
    return static_cast<unsigned>(ibeg - i0);
}

auto BlockchainAccountActivity::pipeline(Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();
    last_job_ = work;
    tadiag("ZZZZ pipeline Work:", print(work));

    switch (work) {
        case Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::balance: {
            process_balance(in);
        } break;
        case Work::new_block: {
            process_block(in);
        } break;
        case Work::txid: {
            process_txid(in);
        } break;
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::sync: {
            process_sync(in);
        } break;
        case Work::init: {
            startup();
            finish_startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainAccountActivity::state_machine() noexcept -> int
{

    return load_thread_portion_p() ? SM_BlockchainAccountActivity_fast : SM_off;
}

auto BlockchainAccountActivity::shut_down() noexcept -> void
{
    close_pipeline();
    // TODO MT-34 investigate what other actions might be needed
}

auto BlockchainAccountActivity::print(Work type) noexcept -> const char*
{
    static const auto map = Map<Work, const char*>{
        {Work::shutdown, "shutdown"},
        {Work::contact, "contact"},
        {Work::balance, "balance"},
        {Work::new_block, "new_block"},
        {Work::txid, "txid"},
        {Work::reorg, "reorg"},
        {Work::statechange, "statechange"},
        {Work::sync, "sync"},
        {Work::init, "init"},
        {Work::statemachine, "statemachine"},
    };

    return map.at(type);
}

auto BlockchainAccountActivity::process_balance(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Body();
    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    const auto confirmed = factory::Amount(body.at(2));
    const auto unconfirmed = factory::Amount(body.at(3));
    const auto nym = Widget::api_.Factory().NymID(body.at(4));

    LogConsole()(
        "BlockchainAccountActivity::process_balance confirmed balance: " +
        display_balance(confirmed) + ", unconfirmed balance: " +
        display_balance(unconfirmed) + ", nym: " + nym->asHex())
        .Flush();

    OT_ASSERT(chain_ == chain);
    OT_ASSERT(primary_id_ == nym);

    const auto oldBalance = [&] {
        eLock lock(shared_lock_);

        const auto oldbalance = balance_;
        balance_ = unconfirmed;
        return oldbalance;
    }();
    const auto oldConfirmed = [&] {
        eLock lock(shared_lock_);

        const auto oldconfirmed = confirmed_;
        confirmed_ = confirmed;
        return oldconfirmed;
    }();

    if (oldBalance != unconfirmed) {
        notify_balance(unconfirmed);
    } else if (oldConfirmed != confirmed) {
        UpdateNotify();
    }

    tdiag("ZZZZ process_balance");
    load_thread();
}

auto BlockchainAccountActivity::process_block(const Message& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    process_height(body.at(3).as<blockchain::block::Height>());
}

auto BlockchainAccountActivity::process_contact(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto contactID = Widget::api_.Factory().Identifier(body.at(1))->str();
    const auto txids = [&] {
        auto out = UnallocatedSet<OTData>{};
        for_each_row([&](const auto& row) {
            for (const auto& id : row.Contacts()) {
                if (contactID == id) {
                    out.emplace(
                        blockchain::NumberToHash(Widget::api_, row.UUID()));

                    break;
                }
            }
        });

        return out;
    }();

    for (const auto& txid : txids) { process_txid(txid); }
}

auto BlockchainAccountActivity::process_height(
    const blockchain::block::Height height) noexcept -> void
{
    if (height == height_) { return; }

    height_ = height;
    tdiag("ZZZZ process_height");
    load_thread();
}

auto BlockchainAccountActivity::process_reorg(const Message& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(5 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    process_height(body.at(5).as<blockchain::block::Height>());
}

auto BlockchainAccountActivity::process_state(const Message& in) noexcept
    -> void
{
    {
        const auto body = in.Body();

        OT_ASSERT(2 < body.size());

        const auto chain = body.at(1).as<blockchain::Type>();

        if (chain_ != chain) { return; }

        const auto enabled = body.at(2).as<bool>();

        if (false == enabled) { return; }
    }

    try {
        const auto& chain =
            Widget::api_.Network().Blockchain().GetChain(chain_);
        process_height(chain.HeaderOracle().BestChain().height_);
    } catch (...) {
    }
}

auto BlockchainAccountActivity::process_sync(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto height = body.at(2).as<blockchain::block::Height>();
    const auto target = body.at(3).as<blockchain::block::Height>();

    OT_ASSERT(height <= std::numeric_limits<int>::max());
    OT_ASSERT(target <= std::numeric_limits<int>::max());

    const auto previous = progress_.get_progress();
    const auto current = static_cast<int>(height);
    const auto max = static_cast<int>(target);
    const auto percent = progress_.set(current, max);

    if (progress_.get_progress() != previous) {
        auto lock = Lock{callbacks_.lock_};
        const auto& cb = callbacks_.cb_.sync_;

        if (cb) { cb(current, max, percent); }

        UpdateNotify();
    }
}

auto BlockchainAccountActivity::process_txid(const Message& in) noexcept -> void
{
    wait_for_startup();
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto& api = Widget::api_;
    const auto txid = api.Factory().Data(body.at(1));
    const auto chain = body.at(2).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto proto = proto::Factory<proto::BlockchainTransaction>(body.at(3));
    process_txid(txid, factory::BitcoinTransaction(api, proto));
}

auto BlockchainAccountActivity::process_txid(const Data& txid) noexcept
    -> std::optional<AccountActivityRowID>
{
    PTimer<Diag>::start(Diag::load_transaction_bitcoin);
    auto bct = Widget::api_.Crypto().Blockchain().LoadTransactionBitcoin(txid);
    PTimer<Diag>::stop(Diag::load_transaction_bitcoin);
    PTimer<Diag>::start(Diag::process_txid);
    auto r = process_txid(txid, std::move(bct));
    PTimer<Diag>::stop(Diag::process_txid);
    return r;
}

auto BlockchainAccountActivity::process_txid(
    const Data& txid,
    std::unique_ptr<const blockchain::bitcoin::block::Transaction>
        pTX) noexcept  // TODO : MT-83
    -> std::optional<AccountActivityRowID>
{
    if (!pTX) { return std::nullopt; }
    const auto& tx = pTX->Internal();
    if (!contains(tx.Chains(), chain_)) { return std::nullopt; }

    PTimer<Diag>::start(Diag::blockchain_thread_item_id);
    const auto rowID = AccountActivityRowID{
        blockchain_thread_item_id(Widget::api_.Crypto(), chain_, txid),
        proto::PAYMENTEVENTTYPE_COMPLETE};
    PTimer<Diag>::stop(Diag::blockchain_thread_item_id);

    PTimer<Diag>::start(Diag::process_txid_lambda);
    const auto sortKey{tx.Timestamp()};
    const auto conf = [&]() -> int {
        const auto height = tx.ConfirmationHeight();

        if ((0 > height) || (height > height_)) { return 0; }

        return static_cast<int>(height_ - height) + 1;
    }();
    PTimer<Diag>::stop(Diag::process_txid_lambda);
    PTimer<Diag>::start(Diag::process_txid_custom);
    auto custom = CustomData{
        new proto::PaymentWorkflow(),
        new proto::PaymentEvent(),
        const_cast<void*>(static_cast<const void*>(pTX.release())),
        new blockchain::Type{chain_},
        new UnallocatedCString{
            Widget::api_.Crypto().Blockchain().ActivityDescription(
                primary_id_, chain_, tx)},
        new OTData{tx.ID()},
        new int{conf},
    };
    PTimer<Diag>::stop(Diag::process_txid_custom);

    PTimer<Diag>::start(Diag::add_item);
    add_item(rowID, sortKey, custom);
    PTimer<Diag>::stop(Diag::add_item);
    return std::move(rowID);
}

auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const Amount& amount,
    const UnallocatedCString& memo) const noexcept -> bool
{
    try {
        const auto& network =
            Widget::api_.Network().Blockchain().GetChain(chain_);
        const auto recipient = Widget::api_.Factory().PaymentCode(address);

        if (0 < recipient.Version()) {
            network.SendToPaymentCode(primary_id_, recipient, amount, memo);
        } else {
            network.SendToAddress(primary_id_, address, amount, memo);
        }

        return true;
    } catch (...) {

        return false;
    }
}

auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const UnallocatedCString& amount,
    const UnallocatedCString& memo,
    Scale scale) const noexcept -> bool
{
    try {

        const auto& definition =
            display::GetDefinition(BlockchainToUnit(chain_));
        return Send(address, definition.Import(amount, scale), memo);
    } catch (...) {

        return false;
    }
}

auto BlockchainAccountActivity::startup() noexcept -> void
{
    tdiag("ZZZZ startup");
    load_thread();
}

auto BlockchainAccountActivity::ValidateAddress(
    const UnallocatedCString& in) const noexcept -> bool
{
    {
        const auto code = Widget::api_.Factory().PaymentCode(in);

        if (0 < code.Version()) { return true; }
    }

    using Style = blockchain::crypto::AddressStyle;

    const auto [data, style, chains, supported] =
        Widget::api_.Crypto().Blockchain().DecodeAddress(in);

    if (Style::Unknown == style) { return false; }

    if (0 == chains.count(chain_)) { return false; }

    return supported;
}

auto BlockchainAccountActivity::ValidateAmount(
    const UnallocatedCString& text) const noexcept -> UnallocatedCString
{
    try {

        const auto& definition =
            display::GetDefinition(BlockchainToUnit(chain_));
        return definition.Format(definition.Import(text));
    } catch (...) {

        return {};
    }
}

auto BlockchainAccountActivity::last_job_str() const noexcept -> std::string
{
    return std::string{print(last_job_)};
}

BlockchainAccountActivity::~BlockchainAccountActivity()
{
    wait_for_startup();
    protect_shutdown([this] { shut_down(); });
}
}  // namespace opentxs::ui::implementation
