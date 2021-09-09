// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_HPP
#define OPENTXS_CORE_IDENTIFIER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto

class Cheque;
class Contract;
class Identifier;
class Item;

using OTIdentifier = Pimpl<Identifier>;

OPENTXS_EXPORT auto operator==(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
/** An Identifier is basically a 256 bit hash value. This class makes it easy to
 * convert IDs back and forth to strings. */
class OPENTXS_EXPORT Identifier : virtual public Data
{
public:
    using ot_super = opentxs::Data;

    static auto Random() -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory() -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Identifier& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const std::string& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const String& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const identity::Nym& nym)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Cheque& cheque)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Item& item)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Contract& contract)
        -> opentxs::Pimpl<opentxs::Identifier>;
    OPENTXS_NO_EXPORT static auto Factory(
        const contact::ContactItemType type,
        const proto::HDPath& path) -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Validate(const std::string& id) -> bool;

    using ot_super::operator==;
    virtual auto operator==(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator!=;
    virtual auto operator!=(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator>;
    virtual auto operator>(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator<;
    virtual auto operator<(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator<=;
    virtual auto operator<=(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator>=;
    virtual auto operator>=(const Identifier& rhs) const noexcept -> bool = 0;

    virtual void GetString(String& theStr) const = 0;
    virtual auto Type() const -> const ID& = 0;

    virtual auto CalculateDigest(
        const ReadView bytes,
        const ID type = ID::blake2b) -> bool = 0;
    virtual void SetString(const std::string& encoded) = 0;
    virtual void SetString(const String& encoded) = 0;
    using ot_super::swap;
    virtual void swap(Identifier& rhs) = 0;

    ~Identifier() override = default;

protected:
    Identifier() = default;

private:
    friend opentxs::Pimpl<opentxs::Identifier>;

#ifndef _WIN32
    auto clone() const -> Identifier* override = 0;
#endif
    Identifier(const Identifier&) = delete;
    Identifier(Identifier&&) = delete;
    auto operator=(const Identifier&) -> Identifier& = delete;
    auto operator=(Identifier&&) -> Identifier& = delete;
};
}  // namespace opentxs

namespace std
{
template <>
struct OPENTXS_EXPORT less<opentxs::OTIdentifier> {
    auto operator()(
        const opentxs::OTIdentifier& lhs,
        const opentxs::OTIdentifier& rhs) const -> bool;
};
}  // namespace std
#endif
