// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_SIGNATURE_HPP
#define OPENTXS_CORE_CRYPTO_SIGNATURE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Armored.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class OTSignatureMetadata;
class Signature;

using OTSignature = Pimpl<Signature>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Signature : virtual public Armored
{
public:
    static Pimpl<opentxs::Signature> Factory(const api::internal::Core& api);

    virtual const OTSignatureMetadata& getMetaData() const = 0;

    virtual OTSignatureMetadata& getMetaData() = 0;

    ~Signature() override = default;

protected:
    Signature() = default;

private:
    Signature(const Signature&) = delete;
    Signature(Signature&&) = delete;
    Signature& operator=(const Signature&) = delete;
    Signature& operator=(Signature&&) = delete;
};
}  // namespace opentxs
#endif
