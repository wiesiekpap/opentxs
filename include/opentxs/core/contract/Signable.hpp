// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SIGNABLE_HPP
#define OPENTXS_CORE_CONTRACT_SIGNABLE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace proto
{
class Signature;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
class OPENTXS_EXPORT Signable
{
public:
    using Signature = std::shared_ptr<proto::Signature>;

    virtual auto Alias() const -> std::string = 0;
    virtual auto ID() const -> OTIdentifier = 0;
    virtual auto Name() const -> std::string = 0;
    virtual auto Nym() const -> Nym_p = 0;
    virtual auto Terms() const -> const std::string& = 0;
    virtual auto Serialize() const -> OTData = 0;
    virtual auto Validate() const -> bool = 0;
    virtual auto Version() const -> VersionNumber = 0;

    virtual void SetAlias(const std::string& alias) = 0;

    virtual ~Signable() = default;

protected:
    Signable() noexcept = default;

private:
#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Signable* = 0;
#ifdef _WIN32
private:
#endif

    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;
};
}  // namespace contract
}  // namespace opentxs
#endif
