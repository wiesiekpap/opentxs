# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  ottest
  PRIVATE
    "Bip158.cpp"
    "Bip158.hpp"
    #"bch_filter_1307544.cpp"
    #"bch_filter_1307723.cpp"
)
if(NOT MSVC)
  target_sources(
    ottest
    PRIVATE
      "bch_filter_1307544.cpp"
      "bch_filter_1307723.cpp"
)
endif()

if(OT_BLOCKCHAIN_EXPORT AND NOT MSVC)
  # NOTE the RLP test vectors can not be easily embedded when the compiler is
  # MSVC due to string literal length restrictions
  target_sources(
    ottest
    PRIVATE
      "${opentxs_BINARY_DIR}/tests/ottest/data/blockchain/RLP_raw.cpp"
      "RLP.cpp"
      "RLP.hpp"
  )
  target_link_libraries(
    ottest
    PRIVATE Boost::headers
    PUBLIC Boost::system
  )
endif()
