# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-blockchain-regtest-basic Basic.cpp)

add_opentx_test(
  ottest-blockchain-regtest-block-propagation BlockPropagation.cpp
)
add_opentx_test(ottest-blockchain-regtest-connection Connection.cpp)
add_opentx_test(ottest-blockchain-regtest-connection-tcp ConnectionTCP.cpp)
add_opentx_test(ottest-blockchain-regtest-generate-block Mine.cpp)
add_opentx_test(ottest-blockchain-regtest-payment-code PaymentCode.cpp)
add_opentx_test(ottest-blockchain-regtest-reorg Reorg.cpp)
add_opentx_test(ottest-blockchain-regtest-stress Stress.cpp)

if(NOT WIN32)
  add_opentx_test(ottest-blockchain-regtest-sync-server SyncServer.cpp)
endif()

set_tests_properties(ottest-blockchain-regtest-stress PROPERTIES DISABLED TRUE)

set_tests_properties(
  ottest-blockchain-regtest-payment-code PROPERTIES DISABLED TRUE
)

add_opentx_test(ottest-blockchain-regtest-sync-client SyncClient.cpp)
add_opentx_test(ottest-blockchain-regtest-rename RenameWallet.cpp)
add_opentx_test(ottest-blockchain-regtest-restart Restart.cpp)
add_opentx_test(ottest-blockchain-regtest-restart-end StressRestartAtEnd.cpp)
add_opentx_test(
  ottest-blockchain-regtest-restart-inloop StressRestartInLoop.cpp
)
add_opentx_test(ottest-blockchain-regtest-restart-early RestartEarly.cpp)
add_opentx_test(ottest-blockchain-regtest-simple-transfer SimpleTransfer.cpp)
add_opentx_test(
  ottest-blockchain-regtest-send-to-payment-code SendToPaymentCode.cpp
)

add_opentx_test(
  ottest-blockchain-regtest-round-robin-distribution-transactions
  RoundRobinDistributionTransactions.cpp
)
add_opentx_test(
  ottest-blockchain-regtest-round-robin-recapture-transactions
  RoundRobinRecaptureTransactions.cpp
)
