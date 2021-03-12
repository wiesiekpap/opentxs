# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base58)

add_library(
  opentxs-api-crypto OBJECT
  "Asymmetric.cpp"
  "Asymmetric.hpp"
  "Config.cpp"
  "Config.hpp"
  "Crypto.cpp"
  "Crypto.hpp"
  "Encode.cpp"
  "Encode.hpp"
  "Hash.cpp"
  "Hash.hpp"
  "Symmetric.cpp"
  "Symmetric.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Config.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Encode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Hash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Symmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Util.hpp"
)
target_link_libraries(
  opentxs-api-crypto PRIVATE Boost::headers opentxs::messages
)
target_include_directories(
  opentxs-api-crypto SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OPENSSL_EXPORT)
  target_compile_definitions(
    opentxs-api-crypto PUBLIC OT_CRYPTO_USING_OPENSSL=1
  )
else()
  target_compile_definitions(
    opentxs-api-crypto PUBLIC OT_CRYPTO_USING_OPENSSL=0
  )
endif()

target_sources(opentxs-api-crypto PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-crypto>)
add_dependencies(opentxs-api-crypto generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/crypto
    COMPONENT dev
  )
endif()
