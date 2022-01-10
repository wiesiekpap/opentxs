# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "AccountCurrency.cpp"
    "AccountCurrency.hpp"
    "AccountTree.cpp"
    "AccountTree.hpp"
    "AccountTreeItem.cpp"
    "AccountTreeItem.hpp"
    "BlockchainAccountTreeItem.cpp"
    "BlockchainAccountTreeItem.hpp"
    "CustodialAccountTreeItem.cpp"
    "CustodialAccountTreeItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/AccountCurrency.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/AccountTree.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/AccountTreeItem.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/interface/ui
    COMPONENT dev
  )
endif()
