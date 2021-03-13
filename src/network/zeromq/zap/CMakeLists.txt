# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-network-zeromq-zap OBJECT
  "Callback.cpp"
  "Callback.hpp"
  "Handler.cpp"
  "Handler.hpp"
  "Reply.cpp"
  "Reply.hpp"
  "Request.cpp"
  "Request.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/Callback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/Handler.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/Reply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/ZAP.hpp"
)
target_link_libraries(
  opentxs-network-zeromq-zap PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_sources(opentxs-network-zeromq-zap PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-network-zeromq-zap>)
add_dependencies(opentxs-network-zeromq-zap generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/zap
    COMPONENT dev
  )
endif()
