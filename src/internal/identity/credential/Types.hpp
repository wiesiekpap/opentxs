// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::identity::credential
{
using CredentialModeFlag = bool;
static const CredentialModeFlag PRIVATE_VERSION = true;
static const CredentialModeFlag PUBLIC_VERSION = false;
}  // namespace opentxs::identity::credential
