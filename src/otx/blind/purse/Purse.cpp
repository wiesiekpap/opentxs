// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "otx/blind/purse/Purse.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/api/crypto/Symmetric.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/otx/blind/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/PurseType.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/TokenState.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Envelope.pb.h"
#include "serialization/protobuf/Purse.pb.h"

namespace opentxs::otx::blind
{
auto Purse::Imp::at(const std::size_t) const -> const Token&
{
    static const auto blank = Token{};

    return blank;
}

auto Purse::Imp::at(const std::size_t) -> Token&
{
    static auto blank = Token{};

    return blank;
}

auto Purse::Imp::Notary() const -> const identifier::Server&
{
    static const auto id = factory::IdentifierNotary();

    return *id;
}

auto Purse::Imp::PrimaryKey(PasswordPrompt&) -> crypto::key::Symmetric&
{
    static auto blank = crypto::key::blank::Symmetric{};

    return blank;
}

auto Purse::Imp::SecondaryKey(const identity::Nym&, PasswordPrompt&)
    -> const crypto::key::Symmetric&
{
    static auto blank = crypto::key::blank::Symmetric{};

    return blank;
}

auto Purse::Imp::Unit() const -> const identifier::UnitDefinition&
{
    static const auto id = factory::IdentifierUnit();

    return *id;
}

auto Purse::Imp::Value() const -> const Amount&
{
    static auto blank = Amount{};

    return blank;
}
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
Purse::Purse(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

Purse::Purse() noexcept
    : Purse(std::make_unique<Imp>().release())
{
    OT_ASSERT(this == imp_->parent_);
}

Purse::Purse(const Purse& rhs) noexcept
    : Purse(rhs.imp_->clone())
{
    OT_ASSERT(this == imp_->parent_);
}

Purse::Purse(Purse&& rhs) noexcept
    : Purse()
{
    swap(rhs);

    OT_ASSERT(this == imp_->parent_);
    OT_ASSERT(nullptr != rhs.imp_->parent_);
}

Purse::operator bool() const noexcept { return imp_->IsValid(); }

auto Purse::AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
    -> bool
{
    return imp_->AddNym(nym, reason);
}

auto Purse::at(const std::size_t position) -> Token&
{
    return imp_->at(position);
}

auto Purse::at(const std::size_t position) const -> const Token&
{
    return imp_->at(position);
}

auto Purse::begin() const noexcept -> const_iterator { return imp_->cbegin(); }

auto Purse::begin() noexcept -> iterator { return imp_->begin(); }

auto Purse::cbegin() const noexcept -> const_iterator { return imp_->cbegin(); }

auto Purse::cend() const noexcept -> const_iterator { return imp_->cend(); }

auto Purse::EarliestValidTo() const -> Time { return imp_->EarliestValidTo(); }

auto Purse::end() const noexcept -> const_iterator { return imp_->cend(); }

auto Purse::end() noexcept -> iterator { return imp_->end(); }

auto Purse::Internal() const noexcept -> const internal::Purse&
{
    return *imp_;
}

auto Purse::Internal() noexcept -> internal::Purse& { return *imp_; }

auto Purse::IsUnlocked() const -> bool { return imp_->IsUnlocked(); }

auto Purse::LatestValidFrom() const -> Time { return imp_->LatestValidFrom(); }

auto Purse::Notary() const -> const identifier::Server&
{
    return imp_->Notary();
}

auto Purse::operator=(const Purse& rhs) noexcept -> Purse&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = rhs.imp_->clone();
    imp_->parent_ = this;

    return *this;
}

auto Purse::operator=(Purse&& rhs) noexcept -> Purse&
{
    swap(rhs);

    return *this;
}

auto Purse::Pop() -> Token { return imp_->Pop(); }

auto Purse::Push(Token&& token, const PasswordPrompt& reason) -> bool
{
    return imp_->Push(std::move(token), reason);
}

auto Purse::Serialize(AllocateOutput destination) const noexcept -> bool
{
    return imp_->Serialize(std::move(destination));
}

auto Purse::size() const noexcept -> std::size_t { return imp_->size(); }

auto Purse::State() const -> blind::PurseType { return imp_->State(); }

auto Purse::swap(Purse& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);

    OT_ASSERT(this == imp_->parent_);
    OT_ASSERT(nullptr != rhs.imp_->parent_);
}

auto Purse::Type() const -> blind::CashType { return imp_->Type(); }

auto Purse::Unit() const -> const identifier::UnitDefinition&
{
    return imp_->Unit();
}

auto Purse::Unlock(const identity::Nym& nym, const PasswordPrompt& reason) const
    -> bool
{
    return imp_->Unlock(nym, reason);
}

auto Purse::Value() const -> const Amount& { return imp_->Value(); }

auto Purse::Verify(const api::session::Notary& server) const -> bool
{
    return imp_->Verify(server);
}

Purse::~Purse()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::otx::blind
