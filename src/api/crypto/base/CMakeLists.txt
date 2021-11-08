# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-api-crypto-base OBJECT "Crypto.cpp" "Crypto.hpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Crypto.hpp"
)
target_link_libraries(
  opentxs-api-crypto-base PRIVATE Boost::headers opentxs-common
)
target_include_directories(
  opentxs-api-crypto-base SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(LIBSECP256K1_EXPORT)
  target_sources(opentxs-api-crypto-base PRIVATE "Libsecp256k1.cpp")
else()
  target_sources(opentxs-api-crypto-base PRIVATE "noLibsecp256k1.cpp")
endif()

if(OPENSSL_EXPORT)
  target_sources(opentxs-api-crypto-base PRIVATE "OpenSSL.cpp")
else()
  target_sources(opentxs-api-crypto-base PRIVATE "noOpenSSL.cpp")
endif()

target_sources(opentxs-api-crypto-base PRIVATE ${cxx-install-headers})

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-api-crypto-base
    REUSE_FROM
    opentxs-common
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-crypto-base>)
add_dependencies(opentxs-api-crypto-base generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/crypto
    COMPONENT dev
  )
endif()
