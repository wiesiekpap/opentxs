# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(SECP256K1_EXPORT AND BIP32_EXPORT)
  add_opentx_test(unittests-opentxs-ui-contactlist Test_ContactList.cpp)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  add_opentx_test(
    unittests-opentxs-ui-blockchainselector Test_BlockchainSelector.cpp
  )
endif()

add_opentx_test(unittests-opentxs-ui-items Test_Items.cpp)
