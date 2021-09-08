# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-blockchainaccountstatus OBJECT
  "BlockchainAccountStatus.cpp"
  "BlockchainAccountStatus.hpp"
  "BlockchainSubaccount.cpp"
  "BlockchainSubaccount.hpp"
  "BlockchainSubaccountSource.cpp"
  "BlockchainSubaccountSource.hpp"
  "BlockchainSubchain.cpp"
  "BlockchainSubchain.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainAccountStatus.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainSubaccount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainSubaccountSource.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/BlockchainSubchain.hpp"
)
target_link_libraries(
  opentxs-ui-blockchainaccountstatus PRIVATE opentxs::messages
)
target_sources(
  opentxs-ui-blockchainaccountstatus PRIVATE ${cxx-install-headers}
)
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-blockchainaccountstatus>
)
add_dependencies(opentxs-ui-blockchainaccountstatus generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
