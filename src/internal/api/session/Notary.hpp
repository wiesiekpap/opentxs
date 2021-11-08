// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/session/Session.hpp"
#include "opentxs/api/session/Notary.hpp"

namespace opentxs::api::session::internal
{
class Notary : virtual public session::Notary, virtual public Session
{
public:
    ~Notary() override = default;
};
}  // namespace opentxs::api::session::internal
