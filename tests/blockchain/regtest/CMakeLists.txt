# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

add_opentx_test(unittests-opentxs-blockchain-regtest-basic Basic.cpp)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-block-propagation BlockPropagation.cpp
)
add_opentx_test(unittests-opentxs-blockchain-regtest-connection Connection.cpp)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-connection-tcp ConnectionTCP.cpp
)
add_opentx_test(unittests-opentxs-blockchain-regtest-generate-block Mine.cpp)
add_opentx_test(
  unittests-opentxs-blockchain-regtest-payment-code PaymentCode.cpp
)
add_opentx_test(unittests-opentxs-blockchain-regtest-reorg Reorg.cpp)
add_opentx_test(unittests-opentxs-blockchain-regtest-stress Stress.cpp)

if(NOT WIN32)
  add_opentx_test(
    unittests-opentxs-blockchain-regtest-sync-server SyncServer.cpp
  )
endif()

set_tests_properties(
  unittests-opentxs-blockchain-regtest-stress PROPERTIES DISABLED TRUE
)

set_tests_properties(
  unittests-opentxs-blockchain-regtest-payment-code PROPERTIES DISABLED TRUE
)
