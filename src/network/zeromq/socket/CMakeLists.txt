# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Raw.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Socket.hpp"
    "Bidirectional.hpp"
    "Bidirectional.tpp"
    "Dealer.cpp"
    "Dealer.hpp"
    "Pair.cpp"
    "Pair.hpp"
    "Pipeline.cpp"
    "Pipeline.hpp"
    "Publish.cpp"
    "Publish.hpp"
    "Pull.cpp"
    "Pull.hpp"
    "Push.cpp"
    "Push.hpp"
    "Raw.cpp"
    "Raw.hpp"
    "Receiver.hpp"
    "Receiver.tpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
    "Router.cpp"
    "Router.hpp"
    "Sender.hpp"
    "Sender.tpp"
    "Socket.cpp"
    "Socket.hpp"
    "Subscribe.cpp"
    "Subscribe.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Dealer.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Pair.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Publish.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Pull.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Push.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Reply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Router.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Sender.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Socket.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/SocketType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Subscribe.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Types.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
          "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)
target_link_libraries(opentxs-common PRIVATE "${OT_ZMQ_TARGET}")
target_link_libraries(opentxs PRIVATE "${OT_ZMQ_TARGET}")
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/socket
    COMPONENT dev
  )
endif()
