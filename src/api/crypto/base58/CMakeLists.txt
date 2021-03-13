# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-api-crypto-base58 OBJECT "base58.cpp" "base58.h")
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-crypto-base58>)
