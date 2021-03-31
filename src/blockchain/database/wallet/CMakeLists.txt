# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-blockchain-database
  PRIVATE
    "Output.cpp"
    "Output.hpp"
    "Proposal.cpp"
    "Proposal.hpp"
    "Subchain.cpp"
    "Subchain.hpp"
    "Transaction.cpp"
    "Transaction.hpp"
)
target_link_libraries(opentxs-blockchain-database PRIVATE Boost::headers)
target_include_directories(
  opentxs-blockchain-database SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
