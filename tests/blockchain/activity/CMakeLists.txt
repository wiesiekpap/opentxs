# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

add_opentx_test(unittests-opentxs-blockchain-activity-labels Test_Labels.cpp)
add_opentx_test(unittests-opentxs-blockchain-activity-merge Test_Merge.cpp)
add_opentx_test(unittests-opentxs-blockchain-activity-threads Test_Threads.cpp)
add_opentx_test(unittests-opentxs-blockchain-activity-ui Test_UI.cpp)

set_tests_properties(
  unittests-opentxs-blockchain-activity-merge PROPERTIES DISABLED TRUE
)
set_tests_properties(
  unittests-opentxs-blockchain-activity-threads PROPERTIES DISABLED TRUE
)
set_tests_properties(
  unittests-opentxs-blockchain-activity-ui PROPERTIES DISABLED TRUE
)
