# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Batch.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Context.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Pool.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Thread.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Types.hpp"
    "Batch.cpp"
    "Context.cpp"
    "Context.hpp"
    "Pool.cpp"
    "Pool.hpp"
    "Thread.cpp"
    "Thread.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Context.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
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
