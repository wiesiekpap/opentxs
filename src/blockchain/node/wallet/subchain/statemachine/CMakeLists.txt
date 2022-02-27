# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
    "Batch.cpp"
    "Batch.hpp"
    "BlockIndex.cpp"
    "BlockIndex.hpp"
    "Index.cpp"
    "Index.hpp"
    "Job.cpp"
    "Job.hpp"
    "Mempool.cpp"
    "Mempool.hpp"
    "Process.cpp"
    "Process.hpp"
    "Progress.cpp"
    "Progress.hpp"
    "Rescan.cpp"
    "Rescan.hpp"
    "Scan.cpp"
    "Scan.hpp"
    "Work.cpp"
    "Work.hpp"
)
