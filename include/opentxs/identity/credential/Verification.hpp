// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_CREDENTIAL_VERIFICATION_HPP
#define OPENTXS_IDENTITY_CREDENTIAL_VERIFICATION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/identity/credential/Base.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace proto
{
class Verification;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
namespace credential
{
class OPENTXS_EXPORT Verification : virtual public Base
{
public:
    OPENTXS_NO_EXPORT static auto SigningForm(const proto::Verification& item)
        -> proto::Verification;
    OPENTXS_NO_EXPORT static auto VerificationID(
        const api::Core& api,
        const proto::Verification& item) -> std::string;

    ~Verification() override = default;

protected:
    Verification() noexcept {}  // TODO Signable

private:
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;
};
}  // namespace credential
}  // namespace identity
}  // namespace opentxs
#endif
