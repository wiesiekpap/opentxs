# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(curve)
add_subdirectory(socket)
add_subdirectory(zap)

add_library(
  opentxs-network-zeromq OBJECT
  "Context.cpp"
  "Context.hpp"
  "Frame.cpp"
  "Frame.hpp"
  "FrameIterator.cpp"
  "FrameSection.cpp"
  "ListenCallback.cpp"
  "ListenCallback.hpp"
  "ListenCallbackSwig.cpp"
  "ListenCallbackSwig.hpp"
  "Message.cpp"
  "PairEventCallback.cpp"
  "PairEventCallback.hpp"
  "PairEventCallbackSwig.cpp"
  "PairEventCallbackSwig.hpp"
  "PairEventListener.cpp"
  "PairEventListener.hpp"
  "Proxy.cpp"
  "Proxy.hpp"
  "ReplyCallback.cpp"
  "ReplyCallback.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Context.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Frame.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/FrameIterator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/FrameSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ListenCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ListenCallbackSwig.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Message.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/PairEventCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/PairEventCallbackSwig.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Proxy.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ReplyCallback.hpp"
)
target_link_libraries(
  opentxs-network-zeromq PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_sources(opentxs-network-zeromq PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-zeromq>)
add_dependencies(opentxs-network-zeromq generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq
    COMPONENT dev
  )
endif()
