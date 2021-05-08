// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/rpc/request/Base.hpp"  // IWYU pragma: associated

#include <memory>

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
class GetAccountActivity;
class GetAccountBalance;
class ListAccounts;
class ListNyms;
class SendPayment;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs

namespace opentxs::rpc::request
{
struct Base::Imp {
    const Base* parent_;
    const VersionNumber version_;
    const std::string cookie_;
    const CommandType type_;
    const SessionIndex session_;
    const AssociateNyms associate_nym_;
    const std::string owner_;
    const std::string notary_;
    const std::string unit_;
    const Identifiers identifiers_;

    static auto check_dups(const Identifiers& data, const char* type) noexcept(
        false) -> void;

    virtual auto asGetAccountActivity() const noexcept
        -> const GetAccountActivity&;
    virtual auto asGetAccountBalance() const noexcept
        -> const GetAccountBalance&;
    virtual auto asListAccounts() const noexcept -> const ListAccounts&;
    virtual auto asListNyms() const noexcept -> const ListNyms&;
    virtual auto asSendPayment() const noexcept -> const SendPayment&;
    auto check_identifiers() const noexcept(false) -> void;
    auto check_session() const noexcept(false) -> void;
    virtual auto serialize(proto::RPCCommand& dest) const noexcept -> bool;
    auto serialize(AllocateOutput dest) const noexcept -> bool;
    auto serialize_identifiers(proto::RPCCommand& dest) const noexcept -> void;
    auto serialize_notary(proto::RPCCommand& dest) const noexcept -> void;
    auto serialize_owner(proto::RPCCommand& dest) const noexcept -> void;
    auto serialize_unit(proto::RPCCommand& dest) const noexcept -> void;

    Imp(const Base* parent) noexcept;
    Imp(const Base* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const AssociateNyms& nyms) noexcept;
    Imp(const Base* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const Identifiers& identifiers,
        const AssociateNyms& nyms) noexcept;
    Imp(const Base* parent,
        const CommandType& command,
        VersionNumber version,
        SessionIndex session,
        const std::string& owner,
        const std::string& notary,
        const std::string& unit,
        const AssociateNyms& nyms) noexcept;
    Imp(const Base* parent, const proto::RPCCommand& serialized) noexcept;

    virtual ~Imp() = default;

private:
    static auto make_cookie() noexcept -> std::string;

    Imp(const Base* parent,
        VersionNumber version,
        const std::string& cookie,
        const CommandType& type,
        SessionIndex session,
        const AssociateNyms& nyms,
        const std::string& owner,
        const std::string& notary,
        const std::string& unit,
        const Identifiers identifiers) noexcept;
    Imp() = delete;

    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::rpc::request
