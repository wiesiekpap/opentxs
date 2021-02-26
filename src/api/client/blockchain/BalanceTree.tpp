// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "BalanceTree.hpp"  // IWYU pragma: associated

#include "internal/api/client/blockchain/Factory.hpp"

namespace opentxs
{
namespace proto
{
class Bip47Channel;
class HDAccount;
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
template <>
struct BalanceTree::Factory<internal::HD, proto::HDPath, PasswordPrompt> {
    static auto get(
        const api::internal::Core& api,
        const BalanceTree& parent,
        Identifier& id,
        const proto::HDPath& data,
        const PasswordPrompt& reason) noexcept -> std::unique_ptr<internal::HD>
    {
        return factory::BlockchainHDBalanceNode(api, parent, data, reason, id);
    }
};
template <>
struct BalanceTree::Factory<internal::HD, proto::HDAccount> {
    static auto get(
        const api::internal::Core& api,
        const BalanceTree& parent,
        Identifier& id,
        const proto::HDAccount& data) noexcept -> std::unique_ptr<internal::HD>
    {
        return factory::BlockchainHDBalanceNode(api, parent, data, id);
    }
};
template <>
struct BalanceTree::Factory<
    internal::PaymentCode,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    PasswordPrompt> {
    static auto get(
        const api::internal::Core& api,
        const BalanceTree& parent,
        Identifier& id,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<internal::PaymentCode>
    {
        static const auto blank = api.Factory().Data();

        return factory::BlockchainPCBalanceNode(
            api, parent, local, remote, path, blank, reason, id);
    }
};
template <>
struct BalanceTree::Factory<
    internal::PaymentCode,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    opentxs::blockchain::block::Txid,
    PasswordPrompt> {
    static auto get(
        const api::internal::Core& api,
        const BalanceTree& parent,
        Identifier& id,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& txid,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<internal::PaymentCode>
    {
        return factory::BlockchainPCBalanceNode(
            api, parent, local, remote, path, txid, reason, id);
    }
};
template <>
struct BalanceTree::Factory<internal::PaymentCode, proto::Bip47Channel> {
    static auto get(
        const api::internal::Core& api,
        const BalanceTree& parent,
        Identifier& id,
        const proto::Bip47Channel& data) noexcept
        -> std::unique_ptr<internal::PaymentCode>
    {
        return factory::BlockchainPCBalanceNode(api, parent, data, id);
    }
};

template <typename InterfaceType, typename PayloadType>
auto BalanceTree::NodeGroup<InterfaceType, PayloadType>::add(
    const Lock& lock,
    const Identifier& id,
    std::unique_ptr<PayloadType> node) noexcept -> bool
{
    if (false == bool(node)) {
        LogOutput("opentxs::api::client::blockchain::implementation::"
                  "BalanceTree::NodeGroup::")(__FUNCTION__)(": Invalid node")
            .Flush();

        return false;
    }

    if (0 < index_.count(id)) {
        LogOutput(
            "opentxs::api::client::blockchain::implementation::"
            "BalanceTree::NodeGroup::")(__FUNCTION__)(": Index already exists")
            .Flush();

        return false;
    }

    nodes_.emplace_back(std::move(node));
    const auto position = std::size_t{nodes_.size() - 1u};
    index_.emplace(id, position);

    return true;
}
}  // namespace opentxs::api::client::blockchain::implementation
