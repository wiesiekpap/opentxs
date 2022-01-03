# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/BlockchainAccountStatus.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/BlockchainSubaccount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/BlockchainSubaccountSource.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/BlockchainSubchain.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/ui
    COMPONENT dev
  )
endif()
