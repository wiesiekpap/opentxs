# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-integration Test_Basic.cpp)
add_opentx_test(ottest-integration-addcontact Test_AddContact.cpp)
add_opentx_test(ottest-integration-deposit Test_DepositCheques.cpp)
add_opentx_test(ottest-integration-pair Test_Pair.cpp)

set_tests_properties(ottest-integration PROPERTIES DISABLED TRUE)
set_tests_properties(ottest-integration-addcontact PROPERTIES DISABLED TRUE)
set_tests_properties(ottest-integration-deposit PROPERTIES DISABLED TRUE)
set_tests_properties(ottest-integration-pair PROPERTIES DISABLED TRUE)
