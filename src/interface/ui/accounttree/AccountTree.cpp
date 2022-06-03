// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "interface/ui/accounttree/AccountTree.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <exception>
#include <future>
#include <memory>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/Factory.hpp"                // IWYU pragma: keep
#include "internal/core/identifier/Identifier.hpp"  // IWYU pragma: keep
#include "internal/otx/common/Account.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/interface/ui/AccountCurrency.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto AccountTreeModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountTree>
{
    using ReturnType = ui::implementation::AccountTree;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountTree::AccountTree(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : AccountTreeList(api, nymID, cb, false)
    , Worker(api, 100ms)
{
    // TODO monitor for notary nym changes since this may affect custodial
    // account names
    init_executor({
        UnallocatedCString{api.Endpoints().AccountUpdate()},
        UnallocatedCString{api.Endpoints().BlockchainAccountCreated()},
    });
    // TODO this model might never initialize if blockchain support is disabled
    pipeline_.ConnectDealer(api.Endpoints().BlockchainBalance(), [](auto) {
        return MakeWork(Work::init);
    });
}

auto AccountTree::add_children(ChildMap&& map) noexcept -> void
{
    add_items([&] {
        auto rows = ChildDefinitions{};

        for (auto& it : map) {
            auto& [unitType, currencyData] = it;
            auto& [cSortIndex, currencyName, cCustom, accountMap] =
                currencyData;
            rows.emplace_back(
                unitType,
                std::make_pair(cSortIndex, std::move(currencyName)),
                std::move(cCustom),
                [&] {
                    auto out = CustomData{};
                    using Accounts = UnallocatedVector<AccountCurrencyRowData>;
                    auto& data = [&]() -> auto&
                    {
                        auto& ptr = out.emplace_back(
                            std::make_unique<Accounts>().release());

                        OT_ASSERT(1u == out.size());
                        OT_ASSERT(nullptr != ptr);

                        return *reinterpret_cast<Accounts*>(ptr);
                    }
                    ();

                    for (auto& [accountID, accountData] :
                         std::get<3>(it.second)) {
                        auto& [aSortIndex, accountName, accountType, aCustom] =
                            accountData;
                        auto& row = data.emplace_back(
                            std::move(accountID),
                            std::make_tuple(
                                aSortIndex,
                                accountType,
                                std::move(accountName)),
                            std::move(aCustom),
                            CustomData{});
                        LogInsane()(OT_PRETTY_CLASS())("processing account ")(
                            row.id_)
                            .Flush();
                    }

                    return out;
                }());
        }

        return rows;
    }());
}

auto AccountTree::construct_row(
    const AccountTreeRowID& id,
    const AccountTreeSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::AccountCurrencyWidget(
        *this, Widget::api_, id, index, custom);
}

auto AccountTree::Debug() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    auto counter{-1};
    out << "Account tree for " << Owner().str() << '\n';
    auto row = First();

    if (row->Valid()) {
        out << "  * row " << std::to_string(++counter) << ":\n";
        out << row->Debug();

        while (false == row->Last()) {
            row = Next();
            out << "  * row " << std::to_string(++counter) << ":\n";
            out << row->Debug();
        }
    } else {
        out << "  * empty\n";
    }

    return out.str();
}

auto AccountTree::load() noexcept -> void
{
    try {
        auto chains = SubscribeSet{};
        auto map = [&] {
            auto out = ChildMap{};
            load_blockchain(out, chains);
            load_custodial(out);

            return out;
        }();
        add_children(std::move(map));
        subscribe(std::move(chains));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto AccountTree::load_blockchain(ChildMap& out, SubscribeSet& subscribe)
    const noexcept -> void
{
    const auto& blockchain = Widget::api_.Crypto().Blockchain();

    for (const auto& account : blockchain.AccountList(primary_id_)) {
        load_blockchain_account(
            std::move(const_cast<OTIdentifier&>(account)), out, subscribe);
    }
}

auto AccountTree::load_blockchain_account(
    OTIdentifier&& id,
    ChildMap& out,
    SubscribeSet& subscribe) const noexcept -> void
{
    const auto [chain, owner] =
        Widget::api_.Crypto().Blockchain().LookupAccount(id);

    OT_ASSERT(blockchain::Type::Unknown != chain);
    OT_ASSERT(owner == primary_id_);

    load_blockchain_account(std::move(id), chain, out, subscribe);
}

auto AccountTree::load_blockchain_account(
    blockchain::Type chain,
    ChildMap& out,
    SubscribeSet& subscribe) const noexcept -> void
{
    load_blockchain_account(
        OTIdentifier{Widget::api_.Crypto()
                         .Blockchain()
                         .Account(primary_id_, chain)
                         .AccountID()},
        chain,
        {},
        out,
        subscribe);
}

auto AccountTree::load_blockchain_account(
    OTIdentifier&& id,
    blockchain::Type chain,
    ChildMap& out,
    SubscribeSet& subscribe) const noexcept -> void
{
    load_blockchain_account(std::move(id), chain, {}, out, subscribe);
}

auto AccountTree::load_blockchain_account(
    OTIdentifier&& id,
    blockchain::Type chain,
    Amount&& balance,
    ChildMap& out,
    SubscribeSet& subscribe) const noexcept -> void
{
    const auto type = BlockchainToUnit(chain);
    auto& currencyData = [&]() -> auto&
    {
        if (auto it = out.find(type); out.end() != it) {

            return it->second;
        } else {
            auto& data = out[type];
            // TODO set sort index
            std::get<1>(data) = opentxs::print(type);

            return data;
        }
    }
    ();
    auto& accountMap = std::get<3>(currencyData);
    auto& api = Widget::api_;
    // TODO set sort index
    auto [it, added] = accountMap.try_emplace(
        std::move(id),
        0,
        account_name_blockchain(chain),
        AccountType::Blockchain,
        [&] {
            auto out = CustomData{};
            out.reserve(4);
            out.emplace_back(std::make_unique<UnitType>(type).release());
            out.emplace_back(
                std::make_unique<OTUnitID>(blockchain::UnitID(api, chain))
                    .release());
            out.emplace_back(
                std::make_unique<OTNotaryID>(blockchain::NotaryID(api, chain))
                    .release());
            out.emplace_back(
                std::make_unique<Amount>(std::move(balance)).release());

            return out;
        }());

    OT_ASSERT(added);

    LogInsane()(OT_PRETTY_CLASS())("processing blockchain account ")(it->first)
        .Flush();

    subscribe.emplace(chain);
}

auto AccountTree::load_custodial(ChildMap& out) const noexcept -> void
{
    const auto& storage = Widget::api_.Storage();

    for (auto& account : storage.AccountsByOwner(primary_id_)) {
        load_custodial_account(
            std::move(const_cast<OTIdentifier&>(account)), out);
    }
}

auto AccountTree::load_custodial_account(OTIdentifier&& id, ChildMap& out)
    const noexcept -> void
{
    const auto& wallet = Widget::api_.Wallet();
    auto account = wallet.Internal().Account(id);
    const auto& contractID = account.get().GetInstrumentDefinitionID();
    const auto contract = wallet.UnitDefinition(contractID);
    load_custodial_account(
        std::move(id),
        OTUnitID{contractID},
        contract->UnitOfAccount(),
        account.get().GetBalance(),
        account.get().Alias(),
        out);
}

auto AccountTree::load_custodial_account(
    OTIdentifier&& id,
    Amount&& balance,
    ChildMap& out) const noexcept -> void
{
    const auto& wallet = Widget::api_.Wallet();
    auto account = wallet.Internal().Account(id);
    const auto& contractID = account.get().GetInstrumentDefinitionID();
    const auto contract = wallet.UnitDefinition(contractID);
    load_custodial_account(
        std::move(id),
        OTUnitID{contractID},
        contract->UnitOfAccount(),
        std::move(balance),
        account.get().Alias(),
        out);
}

auto AccountTree::load_custodial_account(
    OTIdentifier&& id,
    OTUnitID&& contract,
    UnitType type,
    Amount&& balance,
    UnallocatedCString&& name,
    ChildMap& out) const noexcept -> void
{
    auto& currencyData = [&]() -> auto&
    {
        if (auto it = out.find(type); out.end() != it) {

            return it->second;
        } else {
            auto& data = out[type];
            // TODO set sort index
            std::get<1>(data) = opentxs::print(type);

            return data;
        }
    }
    ();
    auto& accountMap = std::get<3>(currencyData);
    auto& api = Widget::api_;
    auto notaryID = api.Storage().AccountServer(id);
    // TODO set sort index
    auto [it, added] = accountMap.try_emplace(
        std::move(id),
        0,
        account_name_custodial(api, notaryID, contract, std::move(name)),
        AccountType::Custodial,
        [&] {
            auto out = CustomData{};
            out.reserve(4);
            out.emplace_back(std::make_unique<UnitType>(type).release());
            out.emplace_back(
                std::make_unique<OTUnitID>(std::move(contract)).release());
            out.emplace_back(
                std::make_unique<OTNotaryID>(std::move(notaryID)).release());
            out.emplace_back(
                std::make_unique<Amount>(std::move(balance)).release());

            return out;
        }());

    OT_ASSERT(added);

    LogInsane()(OT_PRETTY_CLASS())("processing custodial account ")(it->first)
        .Flush();
}

auto AccountTree::pipeline(Message&& in) noexcept -> void
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

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_.Push(std::move(in));

        return;
    }

    switch (work) {
        case Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case Work::custodial: {
            process_custodial(std::move(in));
        } break;
        case Work::blockchain: {
            process_blockchain(std::move(in));
        } break;
        case Work::balance: {
            process_blockchain_balance(std::move(in));
        } break;
        case Work::init: {
            startup();
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

auto AccountTree::state_machine() noexcept -> bool { return false; }

auto AccountTree::shut_down() noexcept -> void
{
    close_pipeline();
    // TODO MT-34 investigate what other actions might be needed
}

auto AccountTree::process_blockchain(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(4 < body.size());

    const auto nymID = Widget::api_.Factory().NymID(body.at(2));

    if (nymID != primary_id_) {
        LogInsane()(OT_PRETTY_CLASS())("Update does not apply to this widget")
            .Flush();

        return;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    OT_ASSERT(blockchain::Type::Unknown != chain);

    auto chains = SubscribeSet{};
    add_children([&] {
        auto out = ChildMap{};
        load_blockchain_account(chain, out, chains);

        return out;
    }());
    subscribe(std::move(chains));
}

auto AccountTree::process_blockchain_balance(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    const auto& accountID = Widget::api_.Crypto()
                                .Blockchain()
                                .Account(primary_id_, chain)
                                .AccountID();
    auto subscribe = SubscribeSet{};
    add_children([&] {
        auto out = ChildMap{};
        load_blockchain_account(
            OTIdentifier{accountID},
            chain,
            factory::Amount(body.at(3)),
            out,
            subscribe);

        return out;
    }());
}

auto AccountTree::process_custodial(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto& api = Widget::api_;
    auto id = api.Factory().Identifier(body.at(1));
    const auto owner = api.Storage().AccountOwner(id);

    if (owner != primary_id_) { return; }

    add_children([&] {
        auto out = ChildMap{};
        load_custodial_account(
            std::move(id), factory::Amount(body.at(2).Bytes()), out);

        return out;
    }());
}

auto AccountTree::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

auto AccountTree::subscribe(SubscribeSet&& chains) const noexcept -> void
{
    for (const auto chain : chains) {
        pipeline_.Send([&] {
            using Job = api::crypto::blockchain::BalanceOracleJobs;
            auto work = network::zeromq::tagged_message(Job::registration);
            work.AddFrame(chain);
            work.AddFrame(primary_id_);

            return work;
        }());
    }
}

AccountTree::~AccountTree()
{
    wait_for_startup();
    protect_shutdown([this] { shut_down(); });
}
}  // namespace opentxs::ui::implementation
