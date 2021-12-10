// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <vector>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blind/CashType.hpp"
#include "opentxs/blind/Purse.hpp"
#include "opentxs/blind/PurseType.hpp"
#include "opentxs/blind/Token.hpp"
#include "opentxs/blind/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Envelope.pb.h"
#include "serialization/protobuf/Purse.pb.h"

namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session

class Session;
}  // namespace api

namespace blind
{
class Mint;
}  // namespace blind

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace proto
{
class Purse;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blind::implementation
{
class Purse final : public blind::Purse
{
public:
    auto at(const std::size_t position) const -> const Token& final
    {
        return tokens_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, tokens_.size());
    }
    auto EarliestValidTo() const -> Time final { return earliest_valid_to_; }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto IsUnlocked() const -> bool final { return unlocked_; }
    auto LatestValidFrom() const -> Time final { return latest_valid_from_; }
    auto Notary() const -> const identifier::Server& final { return notary_; }
    auto Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) -> bool final;
    auto Serialize(proto::Purse& out) const noexcept -> bool final;
    auto Serialize(AllocateOutput destination) const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return tokens_.size(); }
    auto State() const -> blind::PurseType final { return state_; }
    auto Type() const -> blind::CashType final { return type_; }
    auto Unit() const -> const identifier::UnitDefinition& final
    {
        return unit_;
    }
    auto Unlock(const identity::Nym& nym, const PasswordPrompt& reason) const
        -> bool final;
    auto Verify(const api::session::Notary& server) const -> bool final;
    auto Value() const -> const Amount& final { return total_value_; }

    auto AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
        -> bool final;
    auto at(const std::size_t position) -> Token& final
    {
        return tokens_.at(position);
    }
    auto begin() noexcept -> iterator final { return iterator(this, 0); }
    auto end() noexcept -> iterator final
    {
        return iterator(this, tokens_.size());
    }
    auto GeneratePrototokens(
        const identity::Nym& owner,
        const Mint& mint,
        const Amount& amount,
        const PasswordPrompt& reason) -> bool;
    auto PrimaryKey(PasswordPrompt& password) -> crypto::key::Symmetric& final;
    auto Pop() -> std::shared_ptr<Token> final;
    auto Push(std::shared_ptr<Token> token, const PasswordPrompt& reason)
        -> bool final;
    auto SecondaryKey(const identity::Nym& owner, PasswordPrompt& password)
        -> const crypto::key::Symmetric& final;

    Purse(
        const api::Session& api,
        const identifier::Nym& owner,
        const identifier::Server& server,
        const blind::CashType type,
        const Mint& mint,
        OTSecret&& secondaryKeyPassword,
        std::unique_ptr<const OTSymmetricKey> secondaryKey,
        std::unique_ptr<const OTEnvelope> secondaryEncrypted);
    Purse(
        const api::Session& api,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const blind::CashType type);
    Purse(
        const api::Session& api,
        const VersionNumber version,
        const blind::CashType type,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const blind::PurseType state,
        const Amount& totalValue,
        const Time validFrom,
        const Time validTo,
        const std::vector<OTToken>& tokens,
        const std::shared_ptr<OTSymmetricKey> primary,
        const std::vector<proto::Envelope>& primaryPasswords,
        const std::shared_ptr<const OTSymmetricKey> secondaryKey,
        const std::shared_ptr<const OTEnvelope> secondaryEncrypted,
        std::optional<OTSecret> secondaryKeyPassword);
    Purse(const api::Session& api, const Purse& owner);
    Purse(const api::Session& api, const proto::Purse& serialized);
    Purse(const api::Session& api, const ReadView& serialized);

    ~Purse() final = default;

private:
    static const opentxs::crypto::key::symmetric::Algorithm mode_;

    const api::Session& api_;
    const VersionNumber version_;
    const blind::CashType type_;
    const OTServerID notary_;
    const OTUnitID unit_;
    blind::PurseType state_;
    Amount total_value_;
    Time latest_valid_from_;
    Time earliest_valid_to_;
    std::vector<OTToken> tokens_;
    mutable bool unlocked_;
    mutable OTSecret primary_key_password_;
    std::shared_ptr<OTSymmetricKey> primary_;
    std::vector<proto::Envelope> primary_passwords_;
    OTSecret secondary_key_password_;
    const std::shared_ptr<const OTSymmetricKey> secondary_;
    const std::shared_ptr<const OTEnvelope> secondary_password_;

    static auto deserialize_secondary_key(
        const api::Session& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const OTSymmetricKey>;
    static auto deserialize_secondary_password(
        const api::Session& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const OTEnvelope>;
    static auto get_passwords(const proto::Purse& in)
        -> std::vector<proto::Envelope>;

    auto clone() const noexcept -> Purse* final { return new Purse(*this); }
    auto generate_key(Secret& password) const -> OTSymmetricKey;

    void apply_times(const Token& token);
    void recalculate_times();

    Purse() = delete;
    Purse(const Purse&);
    Purse(Purse&&) = delete;
    auto operator=(const Purse&) -> Purse& = delete;
    auto operator=(Purse&&) -> Purse& = delete;
};
}  // namespace opentxs::blind::implementation
