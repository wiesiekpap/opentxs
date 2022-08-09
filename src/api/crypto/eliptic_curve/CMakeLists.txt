# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
        opentxs-common
        PRIVATE
        "${opentxs_SOURCE_DIR}/src/internal/api/crypto/eliptic_curve/Common.hpp"
        "${opentxs_SOURCE_DIR}/src/internal/api/crypto/eliptic_curve/Secp256k1.hpp"
        "Common.cpp"
        "Secp256k1.cpp"
)
