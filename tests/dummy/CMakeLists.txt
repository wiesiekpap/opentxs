# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-testframework-simpleTest Test_SimpleTest.cpp)
add_opentx_test(ottest-testframework-fixtureTest Test_FixtureTest.cpp)
add_opentx_test(ottest-testframework-parametrizedTest Test_ParametrizedTest.cpp)
add_opentx_test(ottest-testframework-strictMock Test_StrictMock.cpp)
add_opentx_test(ottest-testframework-niceMock Test_NiceMock.cpp)
add_opentx_test(ottest-testframework-naggyMock Test_NaggyMock.cpp)
