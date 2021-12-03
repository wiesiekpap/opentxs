# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(context)
add_subdirectory(curve)
add_subdirectory(message)
add_subdirectory(socket)
add_subdirectory(zap)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Factory.hpp"
    "ListenCallback.cpp"
    "ListenCallback.hpp"
    "PairEventCallback.cpp"
    "PairEventCallback.hpp"
    "PairEventListener.cpp"
    "PairEventListener.hpp"
    "Proxy.cpp"
    "Proxy.hpp"
    "ReplyCallback.cpp"
    "ReplyCallback.hpp"
    "ZeroMQ.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ListenCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/PairEventCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Proxy.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ReplyCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ZeroMQ.hpp"
)
target_link_libraries(opentxs-common PRIVATE "${OT_ZMQ_TARGET}")
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq
    COMPONENT dev
  )
endif()
