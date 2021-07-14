# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

add_opentx_test(unittests-opentxs-network-zeromq-dealer Test_DealerSocket.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-dealerreply Test_DealerReply.cpp
)
add_opentx_test(
  unittests-opentxs-network-zeromq-dealerrouter Test_DealerRouter.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-frame Test_Frame.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-frameinterator Test_FrameIterator.cpp
)
add_opentx_test(
  unittests-opentxs-network-zeromq-framesection Test_FrameSection.cpp
)
add_opentx_test(
  unittests-opentxs-network-zeromq-listencallback Test_ListenCallback.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-message Test_Message.cpp)
add_opentx_test(unittests-opentxs-network-zeromq-pair Test_PairSocket.cpp)
add_opentx_test(unittests-opentxs-network-zeromq-publish Test_PublishSocket.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-publishsubscribe Test_PublishSubscribe.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-pushpull Test_PushPull.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-pushsubscribe Test_PushSubscribe.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-reply Test_ReplySocket.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-replycallback Test_ReplyCallback.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-request Test_RequestSocket.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-requestreply Test_RequestReply.cpp
)
add_opentx_test(
  unittests-opentxs-network-zeromq-requestrouter Test_RequestRouter.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-router Test_RouterSocket.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-routerdealer Test_RouterDealer.cpp
)
add_opentx_test(
  unittests-opentxs-network-zeromq-routerrouter Test_RouterRouter.cpp
)
add_opentx_test(unittests-opentxs-network-zeromq-stress Test_Stress.cpp)
add_opentx_test(
  unittests-opentxs-network-zeromq-subscribe Test_SubscribeSocket.cpp
)

set_tests_properties(
  unittests-opentxs-network-zeromq-stress PROPERTIES DISABLED TRUE
)
