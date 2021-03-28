# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(
  unittests-opentxs-blockchain-regtest-block-propagation
  Test_block_propagation.cpp
)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-connection Test_connection.cpp
)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-generate-block Test_mine_block.cpp
)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-payment-code Test_payment_code.cpp
)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-send-receive-hd Test_send_hd.cpp
)
add_opentx_test(unittests-opentxs-blockchain-regtest-stress Test_hd_stress.cpp)

if(NOT WIN32)
  add_opentx_test(
    unittests-opentxs-blockchain-regtest-sync-server Test_sync_server.cpp
  )
endif()

set_tests_properties(
  unittests-opentxs-blockchain-regtest-stress PROPERTIES DISABLED TRUE
)
