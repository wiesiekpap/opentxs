// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_REQUEST_HPP
#define OPENTXS_OTX_REQUEST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace otx
{
class Request;
}  // namespace otx

namespace proto
{
class ServerRequest;
}  // namespace proto

using OTXRequest = Pimpl<otx::Request>;
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
class OPENTXS_EXPORT Request : virtual public opentxs::contract::Signable
{
public:
    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    static Pimpl<opentxs::otx::Request> Factory(
        const api::internal::Core& api,
        const Nym_p signer,
        const identifier::Server& server,
        const otx::ServerRequestType type,
        const RequestNumber number,
        const PasswordPrompt& reason);
    OPENTXS_NO_EXPORT static Pimpl<opentxs::otx::Request> Factory(
        const api::internal::Core& api,
        const proto::ServerRequest serialized);
    static Pimpl<opentxs::otx::Request> Factory(
        const api::internal::Core& api,
        const ReadView& view);

    virtual const identifier::Nym& Initiator() const = 0;
    virtual RequestNumber Number() const = 0;
    using Signable::Serialize;
    virtual bool Serialize(AllocateOutput destination) const = 0;
    OPENTXS_NO_EXPORT virtual bool Serialize(
        proto::ServerRequest& serialized) const = 0;
    virtual const identifier::Server& Server() const = 0;
    virtual otx::ServerRequestType Type() const = 0;

    virtual bool SetIncludeNym(
        const bool include,
        const PasswordPrompt& reason) = 0;

    ~Request() override = default;

protected:
    Request() = default;

private:
    friend OTXRequest;

#ifndef _WIN32
    Request* clone() const noexcept override = 0;
#endif

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = delete;
};
}  // namespace otx
}  // namespace opentxs
#endif
