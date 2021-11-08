// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/common/Contract.hpp"

namespace opentxs
{
class Identifier;
class String;
}  // namespace opentxs

namespace opentxs::otx::internal
{
class Contract
{
public:
    virtual auto CalculateContractID(Identifier& out) const noexcept
        -> void = 0;
    virtual auto GetFilename(String& out) const noexcept -> void = 0;
    virtual auto SaveContractRaw(String& out) const noexcept -> bool = 0;

    virtual auto LoadContractFromString(const String& in) noexcept -> bool = 0;
    virtual auto ReleaseSignatures() noexcept -> void = 0;
    virtual auto SaveContract() noexcept -> bool = 0;
    virtual auto SaveContract(
        const char* szFoldername,
        const char* szFilename) noexcept -> bool = 0;

    virtual ~Contract() = default;
};
}  // namespace opentxs::otx::internal
