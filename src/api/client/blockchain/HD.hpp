// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/blockchain/HD.cpp"

#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "api/client/blockchain/BalanceNode.hpp"
#include "api/client/blockchain/Deterministic.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/protobuf/HDAccount.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto

class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class HD final : public internal::HD, public Deterministic
{
public:
    using Element = implementation::BalanceNode::Element;
    using SerializedType = proto::HDAccount;

    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey final;

    HD(const api::internal::Core& api,
       const internal::BalanceTree& parent,
       const proto::HDPath& path,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);
    HD(const api::internal::Core& api,
       const internal::BalanceTree& parent,
       const SerializedType& serialized,
       Identifier& id)
    noexcept(false);

    ~HD() final = default;

private:
    static const VersionNumber DefaultVersion{1};

    VersionNumber version_;
    mutable std::unique_ptr<opentxs::crypto::key::HD> cached_internal_;
    mutable std::unique_ptr<opentxs::crypto::key::HD> cached_external_;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto save(const rLock& lock) const noexcept -> bool final;

    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
