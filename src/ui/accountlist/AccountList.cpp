// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "ui/accountlist/AccountList.hpp"  // IWYU pragma: associated

#include <future>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "internal/core/Core.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "ui/base/List.hpp"
#include "util/Blank.hpp"

#define OT_METHOD "opentxs::ui::implementation::AccountList::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto AccountListModel(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountList>
{
    using ReturnType = ui::implementation::AccountList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountList::AccountList(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : AccountListList(api, nymID, cb, false)
    , Worker(api, {})
#if OT_BLOCKCHAIN
    , blockchain_balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { pipeline_->Push(in); }))
    , blockchain_balance_(Widget::api_.Network().ZeroMQ().DealerSocket(
          blockchain_balance_cb_,
          zmq::socket::Socket::Direction::Connect))
#endif  // OT_BLOCKCHAIN
{
#if OT_BLOCKCHAIN
    const auto connected = blockchain_balance_->Start(
        Widget::api_.Endpoints().BlockchainBalance());

    OT_ASSERT(connected);
#endif  // OT_BLOCKCHAIN

    init_executor({
        api.Endpoints().AccountUpdate()
#if OT_BLOCKCHAIN
            ,
            api.Endpoints().BlockchainAccountCreated()
#endif  // OT_BLOCKCHAIN
    });
    pipeline_->Push(MakeWork(Work::init));
}

auto AccountList::construct_row(
    const AccountListRowID& id,
    const AccountListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
#if OT_BLOCKCHAIN
    const auto blockchain{extract_custom<bool>(custom, 0)};
#endif  // OT_BLOCKCHAIN

    return
#if OT_BLOCKCHAIN
        (blockchain ? factory::BlockchainAccountListItem
                    : factory::AccountListItem)
#else
        (factory::AccountListItem)
#endif  // OT_BLOCKCHAIN
            (*this, Widget::api_, id, index, custom);
}

auto AccountList::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::custodial: {
            process_account(in);
        } break;
#if OT_BLOCKCHAIN
        case Work::new_blockchain: {
            process_blockchain_account(in);
        } break;
        case Work::updated_blockchain: {
            process_blockchain_balance(in);
        } break;
#endif  // OT_BLOCKCHAIN
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto AccountList::process_account(const Identifier& id) noexcept -> void
{
    auto account = Widget::api_.Wallet().Account(id);
    process_account(id, account.get().GetBalance(), account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance) noexcept -> void
{
    auto account = Widget::api_.Wallet().Account(id);
    process_account(id, balance, account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance,
    const std::string& name) noexcept -> void
{
    auto index = make_blank<AccountListSortKey>::value(Widget::api_);
    auto& [type, notary] = index;
    type = Widget::api_.Storage().AccountUnit(id);
    notary = Widget::api_.Storage().AccountServer(id)->str();
    const auto contract = Widget::api_.Storage().AccountContract(id);
    auto custom = CustomData{};
    custom.emplace_back(new bool{false});
    custom.emplace_back(new Amount{balance});
    custom.emplace_back(new OTUnitID{contract});
    custom.emplace_back(new std::string{name});
    add_item(id, index, custom);
}

auto AccountList::process_account(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    auto accountID = Widget::api_.Factory().Identifier();
    accountID->Assign(body.at(1).Bytes());
    const auto balance = body.at(2).as<Amount>();
    process_account(accountID, balance);
}

#if OT_BLOCKCHAIN
auto AccountList::process_blockchain_account(const Message& message) noexcept
    -> void
{
    if (5 > message.Body().size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        return;
    }

    const auto& nymFrame = message.Body_at(2);
    auto nymID = Widget::api_.Factory().NymID();
    nymID->Assign(nymFrame.Bytes());

    if (nymID != primary_id_) {
        LogTrace(OT_METHOD)(__func__)(": Update does not apply to this widget")
            .Flush();

        return;
    }

    const auto& chainFrame = message.Body_at(1);
    subscribe(chainFrame.as<blockchain::Type>());
}

auto AccountList::process_blockchain_balance(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    [[maybe_unused]] const auto confirmed = body.at(2).as<Amount>();
    const auto unconfirmed = body.at(3).as<Amount>();
    const auto& accountID =
        Widget::api_.Blockchain().Account(primary_id_, chain).AccountID();
    auto index = make_blank<AccountListSortKey>::value(Widget::api_);
    auto& [type, notary] = index;
    type = Translate(chain);
    notary = NotaryID(Widget::api_, chain).str();
    auto custom = CustomData{};
    custom.emplace_back(new bool{true});
    custom.emplace_back(new Amount{unconfirmed});
    custom.emplace_back(new blockchain::Type{chain});
    custom.emplace_back(new std::string{AccountName(chain)});
    add_item(accountID, index, custom);
}
#endif  // OT_BLOCKCHAIN

auto AccountList::startup() noexcept -> void
{
    const auto accounts = Widget::api_.Storage().AccountsByOwner(primary_id_);
    LogDetail(OT_METHOD)(__func__)(": Loading ")(accounts.size())(" accounts.")
        .Flush();

    for (const auto& id : accounts) { process_account(id); }

#if OT_BLOCKCHAIN
    const auto bcAccounts = Widget::api_.Blockchain().AccountList(primary_id_);

    for (const auto& account : bcAccounts) {
        const auto [chain, owner] =
            Widget::api_.Blockchain().LookupAccount(account);

        OT_ASSERT(blockchain::Type::Unknown != chain);
        OT_ASSERT(owner == primary_id_);

        subscribe(chain);
    }
#endif  // OT_BLOCKCHAIN

    finish_startup();
}

#if OT_BLOCKCHAIN
auto AccountList::subscribe(const blockchain::Type chain) const noexcept -> void
{
    auto out = Widget::api_.Network().ZeroMQ().TaggedMessage(
        WorkType::BlockchainBalance);
    out->AddFrame(chain);
    out->AddFrame(primary_id_);
    blockchain_balance_->Send(out);
}
#endif  // OT_BLOCKCHAIN

AccountList::~AccountList()
{
    wait_for_startup();
#if OT_BLOCKCHAIN
    blockchain_balance_->Close();
#endif  // OT_BLOCKCHAIN
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
