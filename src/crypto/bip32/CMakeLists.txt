# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-crypto
  PRIVATE
    "Bip32.cpp"
    "Imp.hpp"
    "Imp.cpp"
)

if(BIP32_EXPORT)
  target_sources(opentxs-crypto PRIVATE "hd.cpp")
else()
  target_sources(opentxs-crypto PRIVATE "no-hd.cpp")
endif()
