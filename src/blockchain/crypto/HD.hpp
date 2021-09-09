// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "Proto.hpp"
#include "blockchain/crypto/Deterministic.hpp"
#include "blockchain/crypto/Element.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
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
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto

class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::crypto::implementation
{
class HD final : public internal::HD, public Deterministic
{
public:
    using Element = implementation::Element;
    using SerializedType = proto::HDAccount;

    auto InternalHD() const noexcept -> internal::HD& final
    {
        return const_cast<HD&>(*this);
    }
    auto Name() const noexcept -> std::string final;
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey final;
    auto Standard() const noexcept -> HDProtocol final { return standard_; }

    HD(const api::Core& api,
       const Account& parent,
       const proto::HDPath& path,
       const HDProtocol standard,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);
    HD(const api::Core& api,
       const Account& parent,
       const SerializedType& serialized,
       Identifier& id)
    noexcept(false);

    ~HD() final = default;

private:
    static const VersionNumber DefaultVersion{1};
    static constexpr auto proto_hd_version_ = VersionNumber{1};

    const HDProtocol standard_;
    VersionNumber version_;
    mutable std::unique_ptr<opentxs::crypto::key::HD> cached_internal_;
    mutable std::unique_ptr<opentxs::crypto::key::HD> cached_external_;
    mutable std::optional<std::string> name_;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto save(const rLock& lock) const noexcept -> bool final;

    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation
