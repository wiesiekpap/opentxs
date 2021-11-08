# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(ed25519)
add_subdirectory(rsa)
add_subdirectory(secp256k1)

add_library(
  opentxs-crypto-key-asymmetric OBJECT
  "Asymmetric.cpp"
  "Asymmetric.hpp"
  "EllipticCurve.cpp"
  "EllipticCurve.hpp"
  "HD.cpp"
  "HD.hpp"
  "Keypair.cpp"
  "Keypair.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Algorithm.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Mode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/asymmetric/Role.hpp"
)
target_link_libraries(opentxs-crypto-key-asymmetric PRIVATE opentxs::messages)
target_include_directories(
  opentxs-crypto-key-asymmetric SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-crypto-key-asymmetric PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-key-asymmetric>)
add_dependencies(opentxs-crypto-key-asymmetric generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/key/asymmetric
    COMPONENT dev
  )
endif()
