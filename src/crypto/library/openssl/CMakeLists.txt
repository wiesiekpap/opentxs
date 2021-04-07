# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-crypto-library-openssl OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/crypto/library/OpenSSL.hpp"
  "OpenSSL.cpp"
  "OpenSSL.hpp"
  "OpenSSL_BIO.cpp"
  "OpenSSL_BIO.hpp"
)
target_link_libraries(
  opentxs-crypto-library-openssl
  PRIVATE
    Boost::headers
    OpenSSL::Crypto
    opentxs-api-crypto
    opentxs::messages
)
target_link_libraries(opentxs PUBLIC OpenSSL::Crypto)

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_link_libraries(opentxs-crypto-library-openssl PRIVATE CRYPT32.LIB)
  target_link_libraries(opentxs PUBLIC CRYPT32.LIB)
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-library-openssl>)
add_dependencies(opentxs-crypto-library-openssl generated_code)
