# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(ED25519_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Ed25519.cpp"
      "Ed25519.hpp"
      "Factory.cpp"
  )

  if(BIP32_EXPORT)
    target_sources(opentxs-common PRIVATE "HD.cpp")
  else()
    target_sources(opentxs-common PRIVATE "noHD.cpp")
  endif()
else()
  target_sources(opentxs-common PRIVATE "noEd25519.cpp")
endif()

target_link_libraries(opentxs-common PRIVATE unofficial-sodium::sodium)
target_link_libraries(opentxs PUBLIC unofficial-sodium::sodium)
