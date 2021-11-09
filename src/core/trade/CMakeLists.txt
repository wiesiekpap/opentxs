# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "OTOffer.cpp"
    "OTMarket.cpp"
    "OTTrade.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTMarket.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTOffer.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTTrade.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/trade
    COMPONENT dev
  )
endif()
