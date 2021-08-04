# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-block-bitcoin OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/bitcoin/Bitcoin.hpp"
  "Bitcoin.cpp"
  "Block.cpp"
  "Block.hpp"
  "BlockParser.cpp"
  "BlockParser.hpp"
  "Header.cpp"
  "Header.hpp"
  "Input.cpp"
  "Input.hpp"
  "Inputs.cpp"
  "Inputs.hpp"
  "Output.cpp"
  "Output.hpp"
  "OutputCache.cpp"
  "Outputs.cpp"
  "Outputs.hpp"
  "Script.cpp"
  "Script.hpp"
  "Transaction.cpp"
  "Transaction.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Header.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Input.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Inputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Output.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Outputs.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Script.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Transaction.hpp"
)

target_link_libraries(
  opentxs-blockchain-block-bitcoin PRIVATE Boost::headers opentxs::messages
)
target_sources(opentxs-blockchain-block-bitcoin PRIVATE ${cxx-install-headers})
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-block-bitcoin>
)
add_dependencies(opentxs-blockchain-block-bitcoin generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/block/bitcoin
    COMPONENT dev
  )
endif()
