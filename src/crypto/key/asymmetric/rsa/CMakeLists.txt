# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-crypto-key-asymmetric-rsa OBJECT "")

if(RSA_EXPORT)
  target_sources(
    opentxs-crypto-key-asymmetric-rsa
    PRIVATE
      "Factory.cpp"
      "RSA.cpp"
      "RSA.hpp"
  )
else()
  target_sources(opentxs-crypto-key-asymmetric-rsa PRIVATE "noRSA.cpp")
endif()

target_link_libraries(
  opentxs-crypto-key-asymmetric-rsa PRIVATE opentxs::messages
)

if(OPENSSL_EXPORT)
  target_link_libraries(
    opentxs-crypto-key-asymmetric-rsa PRIVATE OpenSSL::Crypto
  )
  target_link_libraries(opentxs PUBLIC OpenSSL::Crypto)
endif()

target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-crypto-key-asymmetric-rsa>
)
add_dependencies(opentxs-crypto-key-asymmetric-rsa generated_code)
