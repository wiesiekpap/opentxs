# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-network-zeromq-socket OBJECT
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
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Request.tpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Router.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Sender.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Sender.tpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Socket.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Subscribe.hpp"
)
target_link_libraries(
  opentxs-network-zeromq-socket PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PRIVATE "${OT_ZMQ_TARGET}")
target_sources(opentxs-network-zeromq-socket PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-zeromq-socket>)
add_dependencies(opentxs-network-zeromq-socket generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/socket
    COMPONENT dev
  )
endif()
