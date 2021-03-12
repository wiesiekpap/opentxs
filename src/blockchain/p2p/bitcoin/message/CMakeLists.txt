# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-blockchain-p2p-bitcoin-message OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/message/Message.hpp"
  "Addr.cpp"
  "Addr.hpp"
  "Block.cpp"
  "Block.hpp"
  "Blocktxn.cpp"
  "Blocktxn.hpp"
  "Cfcheckpt.cpp"
  "Cfcheckpt.hpp"
  "Cfheaders.cpp"
  "Cfheaders.hpp"
  "Cfilter.cpp"
  "Cfilter.hpp"
  "Cmpctblock.cpp"
  "Cmpctblock.hpp"
  "Feefilter.cpp"
  "Feefilter.hpp"
  "Filteradd.cpp"
  "Filteradd.hpp"
  "Filterload.cpp"
  "Filterload.hpp"
  "Getblocks.cpp"
  "Getblocks.hpp"
  "Getblocktxn.cpp"
  "Getblocktxn.hpp"
  "Getcfcheckpt.cpp"
  "Getcfcheckpt.hpp"
  "Getcfheaders.cpp"
  "Getcfheaders.hpp"
  "Getcfilters.cpp"
  "Getcfilters.hpp"
  "Getdata.cpp"
  "Getdata.hpp"
  "Getheaders.cpp"
  "Getheaders.hpp"
  "Headers.cpp"
  "Headers.hpp"
  "Inv.cpp"
  "Inv.hpp"
  "Merkleblock.cpp"
  "Merkleblock.hpp"
  "Message.cpp"
  "Nopayload.cpp"
  "Nopayload.hpp"
  "Notfound.cpp"
  "Notfound.hpp"
  "Ping.cpp"
  "Ping.hpp"
  "Pong.cpp"
  "Pong.hpp"
  "Reject.cpp"
  "Reject.hpp"
  "Sendcmpct.cpp"
  "Sendcmpct.hpp"
  "Tx.cpp"
  "Tx.hpp"
  "Version.cpp"
  "Version.hpp"
)
target_link_libraries(
  opentxs-blockchain-p2p-bitcoin-message PRIVATE Boost::headers
                                                 opentxs::messages
)
target_sources(
  opentxs PRIVATE $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin-message>
)
add_dependencies(opentxs-blockchain-p2p-bitcoin-message generated_code)
