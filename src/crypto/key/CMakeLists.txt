# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Asymmetric.cpp
    EllipticCurve.cpp
    HD.cpp
    Keypair.cpp
    Symmetric.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/EllipticCurve.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Ed25519.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/HD.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Keypair.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/RSA.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Secp256k1.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Symmetric.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/crypto/key/Factory.hpp"
    Asymmetric.hpp
    EllipticCurve.hpp
    HD.hpp
    Keypair.hpp
    Null.hpp
    Symmetric.hpp
    SymmetricNull.hpp
)

if(RSA_EXPORT)
  list(APPEND cxx-sources RSA.cpp)
  list(APPEND cxx-headers RSA.hpp)
endif()

if(ED25519_EXPORT)
  list(APPEND cxx-sources Ed25519.cpp)
  list(APPEND cxx-headers Ed25519.hpp)
endif()

if(SECP256K1_EXPORT)
  list(APPEND cxx-sources Secp256k1.cpp)
  list(APPEND cxx-headers Secp256k1.hpp)
endif()

add_library(opentxs-crypto-key OBJECT ${cxx-sources} ${cxx-headers})

if(OPENSSL_EXPORT)
  target_include_directories(
    opentxs-crypto-key SYSTEM PRIVATE "${OPENSSL_INCLUDE_DIR}"
  )
endif()

target_link_libraries(
  opentxs-crypto-key PRIVATE opentxs::messages unofficial-sodium::sodium
)
add_dependencies(opentxs-crypto-key generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto/key
    COMPONENT dev
  )
endif()
