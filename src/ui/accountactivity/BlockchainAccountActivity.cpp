// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated

#include <atomic>
#include <functional>
#include <future>
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <type_traits>

#include "display/Definition.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainAccountActivity::"

namespace opentxs::factory
{
auto BlockchainAccountActivityModel(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>
{
    using ReturnType = ui::implementation::BlockchainAccountActivity;
    const auto [chain, owner] = api.Blockchain().LookupAccount(accountID);

    OT_ASSERT(owner == nymID);

    return std::make_unique<ReturnType>(api, chain, nymID, accountID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountActivity::BlockchainAccountActivity(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback& cb) noexcept
    : AccountActivity(
          api,
          nymID,
          accountID,
          AccountType::Blockchain,
          cb,
          display::Definition{
              blockchain::params::Data::Chains().at(chain).scales_})
    , chain_(chain)
    , confirmed_(0)
    , balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { pipeline_->Push(in); }))
    , balance_socket_(Widget::api_.Network().ZeroMQ().DealerSocket(
          balance_cb_,
          zmq::socket::Socket::Direction::Connect))
    , progress_()
{
    const auto connected =
        balance_socket_->Start(Widget::api_.Endpoints().BlockchainBalance());

    OT_ASSERT(connected);

    init(
        {api.Endpoints().BlockchainTransactions(),
         api.Endpoints().BlockchainTransactions(nymID),
         api.Endpoints().BlockchainSyncProgress(),
         api.Endpoints().ContactUpdate()});

    {
        const auto& socket = balance_socket_.get();
        auto work = socket.Context().TaggedMessage(WorkType::BlockchainBalance);
        work->AddFrame(chain_);
        work->AddFrame(nymID);
        socket.Send(work);
    }
}

auto BlockchainAccountActivity::DepositAddress(
    const blockchain::Type chain) const noexcept -> std::string
{
    if ((blockchain::Type::Unknown != chain) && (chain_ != chain)) {
        return {};
    }

    const auto& wallet = Widget::api_.Blockchain().Account(primary_id_, chain_);
    const auto reason = Widget::api_.Factory().PasswordPrompt(
        "Calculating next deposit address");
    const auto& styles = blockchain::params::Data::Chains().at(chain_).styles_;

    if (0u == styles.size()) { return {}; }

    return wallet.GetDepositAddress(styles.front().first, reason);
}

auto BlockchainAccountActivity::display_balance(
    opentxs::Amount value) const noexcept -> std::string
{
    return blockchain::internal::Format(chain_, value);
}

auto BlockchainAccountActivity::load_thread() noexcept -> void
{
    const auto transactions =
        Widget::api_.Storage().BlockchainTransactionList(primary_id_);
    auto active = std::set<AccountActivityRowID>{};

    for (const auto& txid : transactions) {
        if (const auto id = process_txid(txid); id.has_value()) {
            active.emplace(id.value());
        }
    }

    delete_inactive(active);
}

auto BlockchainAccountActivity::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::balance: {
            process_balance(in);
        } break;
        case Work::txid: {
            process_txid(in);
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
            LogOutput(OT_METHOD)(__func__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainAccountActivity::process_balance(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    const auto confirmed = body.at(2).as<Amount>();
    const auto unconfirmed = body.at(3).as<Amount>();
    const auto nym = [&] {
        auto output = Widget::api_.Factory().NymID();
        output->Assign(body.at(4).Bytes());

        return output;
    }();

    OT_ASSERT(chain_ == chain);
    OT_ASSERT(primary_id_ == nym);

    const auto oldBalance = balance_.exchange(unconfirmed);
    const auto oldConfirmed = confirmed_.exchange(confirmed);

    if (oldBalance != unconfirmed) { notify_balance(unconfirmed); }

    if (oldConfirmed != confirmed) { UpdateNotify(); }

    load_thread();
}

auto BlockchainAccountActivity::process_contact(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto contactID = [&] {
        auto id = Widget::api_.Factory().Identifier();
        id->Assign(body.at(1).Bytes());

        return id->str();
    }();
    const auto txids = [&] {
        auto out = std::set<OTData>{};
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

    const auto current = static_cast<int>(height);
    const auto max = static_cast<int>(target);
    const auto percent = progress_.set(current, max);
    Lock lock(callbacks_.lock_);
    const auto& cb = callbacks_.cb_.sync_;

    if (cb) { cb(current, max, percent); }
}

auto BlockchainAccountActivity::process_txid(const Message& in) noexcept -> void
{
    wait_for_startup();
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto txid = Widget::api_.Factory().Data(body.at(1));
    const auto chain = body.at(2).as<blockchain::Type>();

    if (chain != chain_) { return; }

    process_txid(txid);
}

auto BlockchainAccountActivity::process_txid(const Data& txid) noexcept
    -> std::optional<AccountActivityRowID>
{
    const auto rowID = AccountActivityRowID{
        blockchain_thread_item_id(Widget::api_.Crypto(), chain_, txid),
        proto::PAYMENTEVENTTYPE_COMPLETE};
    auto pTX = Widget::api_.Blockchain().LoadTransactionBitcoin(txid);

    if (false == bool(pTX)) { return std::nullopt; }

    const auto& tx = *pTX;

    if (false == contains(tx.Chains(), chain_)) { return std::nullopt; }

    const auto sortKey{tx.Timestamp()};
    auto custom = CustomData{
        new proto::PaymentWorkflow(),
        new proto::PaymentEvent(),
        const_cast<void*>(static_cast<const void*>(pTX.release())),
        new blockchain::Type{chain_},
        new std::string{Widget::api_.Blockchain().ActivityDescription(
            primary_id_, chain_, tx)},
        new OTData{tx.ID()}};
    add_item(rowID, sortKey, custom);

    return std::move(rowID);
}

auto BlockchainAccountActivity::Send(
    const std::string& address,
    const Amount amount,
    const std::string& memo) const noexcept -> bool
{
    try {
        const auto& network =
            Widget::api_.Network().Blockchain().GetChain(chain_);
        const auto recipient = Widget::api_.Factory().PaymentCode(address);

        if (0 < recipient->Version()) {
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
    const std::string& address,
    const std::string& amount,
    const std::string& memo,
    Scale scale) const noexcept -> bool
{
    try {

        return Send(address, scales_.Import(amount, scale), memo);
    } catch (...) {

        return false;
    }
}

auto BlockchainAccountActivity::startup() noexcept -> void { load_thread(); }

auto BlockchainAccountActivity::ValidateAddress(
    const std::string& in) const noexcept -> bool
{
    {
        const auto code = Widget::api_.Factory().PaymentCode(in);

        if (0 < code->Version()) { return true; }
    }

    using Style = blockchain::crypto::AddressStyle;

    const auto [data, style, chains, supported] =
        Widget::api_.Blockchain().DecodeAddress(in);

    if (Style::Unknown == style) { return false; }

    if (0 == chains.count(chain_)) { return false; }

    return supported;
}

auto BlockchainAccountActivity::ValidateAmount(
    const std::string& text) const noexcept -> std::string
{
    try {

        return scales_.Format(scales_.Import(text));
    } catch (...) {

        return {};
    }
}

BlockchainAccountActivity::~BlockchainAccountActivity()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
