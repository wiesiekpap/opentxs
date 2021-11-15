# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(ed25519)
add_subdirectory(rsa)
add_subdirectory(secp256k1)

target_sources(
  opentxs-common
  PRIVATE
    "Asymmetric.cpp"
    "Asymmetric.hpp"
    "EllipticCurve.cpp"
    "EllipticCurve.hpp"
    "HD.cpp"
    "HD.hpp"
    "Keypair.cpp"
    "Keypair.hpp"
)

if(BIP32_EXPORT)
  target_sources(opentxs-common PRIVATE "bip32.cpp")
else()
  target_sources(opentxs-common PRIVATE "no-bip32.cpp")
endif()

target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Algorithm.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Mode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Role.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/key/asymmetric
    COMPONENT dev
  )
endif()
