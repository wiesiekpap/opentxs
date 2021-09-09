// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::crypto::internal
{
struct Asymmetric : virtual public api::crypto::Asymmetric {
    virtual auto API() const noexcept -> const api::Core& = 0;

    ~Asymmetric() override = default;
};
}  // namespace opentxs::api::crypto::internal
