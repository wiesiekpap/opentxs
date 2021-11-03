# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-api-crypto-blockchain OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/api/crypto/Blockchain.hpp"
  "AccountCache.cpp"
  "AccountCache.hpp"
  "Blockchain.cpp"
  "Blockchain.hpp"
  "Imp.cpp"
  "Imp.hpp"
  "Wallets.cpp"
  "Wallets.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Blockchain.hpp"
)
target_link_libraries(
  opentxs-api-crypto-blockchain
  PRIVATE
    bech32
    Boost::headers
    opentxs-common
)
target_include_directories(
  opentxs-api-crypto-blockchain SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
          "${opentxs_SOURCE_DIR}/deps/bech32/ref/c++"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-api-crypto-blockchain
    PRIVATE
      "BalanceOracle.cpp"
      "BalanceOracle.hpp"
      "Imp_blockchain.cpp"
      "Imp_blockchain.hpp"
      "NonNull.cpp"
  )
  target_link_libraries(
    opentxs-api-crypto-blockchain PRIVATE lmdb "${OT_ZMQ_TARGET}"
  )
  target_link_libraries(opentxs PUBLIC lmdb "${OT_ZMQ_TARGET}")
else()
  target_sources(opentxs-api-crypto-blockchain PRIVATE "Null.cpp")
endif()

target_sources(opentxs-api-client PRIVATE ${cxx-install-headers})

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-api-crypto-blockchain
    REUSE_FROM
    opentxs-common
  )
endif()

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-api-crypto-blockchain>)
add_dependencies(opentxs-api-crypto-blockchain generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/crypto
    COMPONENT dev
  )
endif()
