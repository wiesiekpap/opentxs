# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

add_opentx_low_level_test(
  unittests-opentxs-context-client-session Test_ClientSession.cpp
)
add_opentx_low_level_test(unittests-opentxs-context-options Test_Options.cpp)
add_opentx_low_level_test(
  unittests-opentxs-context-startup-shutdown Test_StartupShutdown.cpp
)
add_opentx_low_level_test(
  unittests-opentxs-context-server-session Test_ServerSession.cpp
)
add_opentx_low_level_test(
  unittests-opentxs-context-password-callback Test_PasswordCallback.cpp
)
