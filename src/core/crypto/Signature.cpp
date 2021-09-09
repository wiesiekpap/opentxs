// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "core/crypto/Signature.hpp"  // IWYU pragma: associated

#include "core/Armored.hpp"
#include "opentxs/core/crypto/Signature.hpp"

// template class opentxs::Pimpl<opentxs::Signature>;

namespace opentxs
{
auto Signature::Factory(const api::Core& api) -> OTSignature
{
    return OTSignature(new implementation::Signature(api));
}
}  // namespace opentxs

namespace opentxs::implementation
{
Signature::Signature(const api::Core& api)
    : opentxs::Signature()
    , Armored()
    , metadata_(api)
{
}
}  // namespace opentxs::implementation
