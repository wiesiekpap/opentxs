# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-blockchain-node-bitcoin OBJECT "Bitcoin.cpp" "Bitcoin.hpp")
target_link_libraries(
  opentxs-blockchain-node-bitcoin PRIVATE Boost::headers opentxs::messages
)
target_sources(opentxs-blockchain-node-bitcoin PRIVATE ${cxx-install-headers})
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-node-bitcoin>
)
add_dependencies(opentxs-blockchain-node-bitcoin generated_code)
