// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Account.hpp"  // IWYU pragma: associated

#include "internal/api/Api.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"

namespace opentxs
{
namespace proto
{
class Bip47Channel;
class HDAccount;
class HDPath;
}  // namespace proto

class PaymentCode;
}  // namespace opentxs

namespace opentxs::blockchain::crypto::implementation
{
template <>
struct Account::Factory<crypto::HD, proto::HDPath, HDProtocol, PasswordPrompt> {
    static auto get(
        const api::Core& api,
        const Account& parent,
        Identifier& id,
        const proto::HDPath& data,
        const HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(
            api, parent, data, standard, reason, id);
    }
};
template <>
struct Account::Factory<crypto::HD, proto::HDAccount> {
    static auto get(
        const api::Core& api,
        const Account& parent,
        Identifier& id,
        const proto::HDAccount& data) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(api, parent, data, id);
    }
};
template <>
struct Account::Factory<
    crypto::PaymentCode,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    PasswordPrompt> {
    static auto get(
        const api::Core& api,
        const Account& parent,
        Identifier& id,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        static const auto blank = api.Factory().Data();

        return factory::BlockchainPCSubaccount(
            api, parent, local, remote, path, blank, reason, id);
    }
};
template <>
struct Account::Factory<
    crypto::PaymentCode,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    opentxs::blockchain::block::Txid,
    PasswordPrompt> {
    static auto get(
        const api::Core& api,
        const Account& parent,
        Identifier& id,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& txid,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(
            api, parent, local, remote, path, txid, reason, id);
    }
};
template <>
struct Account::Factory<crypto::PaymentCode, proto::Bip47Channel> {
    static auto get(
        const api::Core& api,
        const Account& parent,
        Identifier& id,
        const proto::Bip47Channel& data) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(api, parent, data, id);
    }
};

template <typename InterfaceType, typename PayloadType>
auto Account::NodeGroup<InterfaceType, PayloadType>::add(
    const Lock& lock,
    const Identifier& id,
    std::unique_ptr<PayloadType> node) noexcept -> bool
{
    if (false == bool(node)) {
        LogOutput("opentxs::blockchain::crypto::implementation::"
                  "Account::NodeGroup::")(__func__)(": Invalid node")
            .Flush();

        return false;
    }

    if (0 < index_.count(id)) {
        LogOutput("opentxs::blockchain::crypto::implementation::"
                  "Account::NodeGroup::")(__func__)(": Index already exists")
            .Flush();

        return false;
    }

    nodes_.emplace_back(std::move(node));
    const auto position = std::size_t{nodes_.size() - 1u};
    index_.emplace(id, position);

    return true;
}
}  // namespace opentxs::blockchain::crypto::implementation
