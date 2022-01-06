// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "core/ui/accountlist/AccountList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <utility>

#include "core/ui/base/List.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"  // IWYU pragma: keep
#include "internal/otx/common/Account.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Blank.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto AccountListModel(
    const api::session::Client& api,
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
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : AccountListList(api, nymID, cb, false)
    , Worker(api, {})
#if OT_BLOCKCHAIN
    , blockchain_balance_cb_(zmq::ListenCallback::Factory(
          [this](auto&& in) { pipeline_.Push(std::move(in)); }))
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
    pipeline_.Push(MakeWork(Work::init));
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
    if (false == running_.load()) { return; }

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
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto AccountList::process_account(const Identifier& id) noexcept -> void
{
    auto account = Widget::api_.Wallet().Internal().Account(id);
    process_account(id, account.get().GetBalance(), account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance) noexcept -> void
{
    auto account = Widget::api_.Wallet().Internal().Account(id);
    process_account(id, balance, account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance,
    const UnallocatedCString& name) noexcept -> void
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
    custom.emplace_back(new UnallocatedCString{name});
    add_item(id, index, custom);
}

auto AccountList::process_account(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto accountID = Widget::api_.Factory().Identifier(body.at(1));
    const auto balance = body.at(2).Bytes();
    process_account(accountID, balance);
}

#if OT_BLOCKCHAIN
auto AccountList::process_blockchain_account(const Message& message) noexcept
    -> void
{
    if (5 > message.Body().size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        return;
    }

    const auto& nymFrame = message.Body_at(2);
    const auto nymID = Widget::api_.Factory().NymID(nymFrame);

    if (nymID != primary_id_) {
        LogTrace()(OT_PRETTY_CLASS())("Update does not apply to this widget")
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
    [[maybe_unused]] const auto confirmed = Amount{body.at(2)};
    const auto unconfirmed = Amount{body.at(3)};
    const auto& accountID = Widget::api_.Crypto()
                                .Blockchain()
                                .Account(primary_id_, chain)
                                .AccountID();
    auto index = make_blank<AccountListSortKey>::value(Widget::api_);
    auto& [type, notary] = index;
    type = BlockchainToUnit(chain);
    notary = NotaryID(Widget::api_, chain).str();
    auto custom = CustomData{};
    custom.emplace_back(new bool{true});
    custom.emplace_back(new Amount{unconfirmed});
    custom.emplace_back(new blockchain::Type{chain});
    custom.emplace_back(new UnallocatedCString{AccountName(chain)});
    add_item(accountID, index, custom);
}
#endif  // OT_BLOCKCHAIN

auto AccountList::startup() noexcept -> void
{
    const auto accounts = Widget::api_.Storage().AccountsByOwner(primary_id_);
    LogDetail()(OT_PRETTY_CLASS())("Loading ")(accounts.size())(" accounts.")
        .Flush();

    for (const auto& id : accounts) { process_account(id); }

#if OT_BLOCKCHAIN
    const auto bcAccounts =
        Widget::api_.Crypto().Blockchain().AccountList(primary_id_);

    for (const auto& account : bcAccounts) {
        const auto [chain, owner] =
            Widget::api_.Crypto().Blockchain().LookupAccount(account);

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
    blockchain_balance_->Send([&] {
        auto work =
            network::zeromq::tagged_message(WorkType::BlockchainBalance);
        work.AddFrame(chain);
        work.AddFrame(primary_id_);

        return work;
    }());
}
#endif  // OT_BLOCKCHAIN

AccountList::~AccountList()
{
    wait_for_startup();
#if OT_BLOCKCHAIN
    blockchain_balance_->Close();
#endif  // OT_BLOCKCHAIN
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
