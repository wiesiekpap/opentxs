# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/trade/OTMarket.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/trade/OTOffer.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/trade/OTTrade.hpp"
    "OTOffer.cpp"
    "OTMarket.cpp"
    "OTTrade.cpp"
)
