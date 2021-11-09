# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Client.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Helpers.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/OTPayment.hpp"
    "DepositPayment.cpp"
    "DepositPayment.hpp"
    "OTPayment.cpp"
    "Operation.cpp"
    "Operation.hpp"
    "PaymentTasks.cpp"
    "PaymentTasks.hpp"
    "StateMachine.cpp"
    "StateMachine.hpp"
    "StateMachine.tpp"
)
