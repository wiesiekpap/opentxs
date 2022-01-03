// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Armored.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/otx/common/crypto/Signature.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs

namespace opentxs::implementation
{
class Signature final : virtual public opentxs::Signature, public Armored
{
public:
    auto getMetaData() const -> const OTSignatureMetadata& final
    {
        return metadata_;
    }
    auto getMetaData() -> OTSignatureMetadata& final { return metadata_; }

    Signature(const api::Session& api);

    ~Signature() final = default;

private:
    friend OTSignature;

    OTSignatureMetadata metadata_;
};
}  // namespace opentxs::implementation
