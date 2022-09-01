# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(SECP256K1_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Factory.cpp"
      "Secp256k1.cpp"
      "Secp256k1.hpp"
  )

  if(BIP32_EXPORT)
    target_sources(opentxs-common PRIVATE "HD.cpp")
  else()
    target_sources(opentxs-common PRIVATE "noHD.cpp")
  endif()
else()
  target_sources(opentxs-common PRIVATE "noSecp256k1.cpp")
endif()
