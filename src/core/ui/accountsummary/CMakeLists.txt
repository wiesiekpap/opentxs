# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "AccountSummary.cpp"
    "AccountSummary.hpp"
    "AccountSummaryItem.cpp"
    "AccountSummaryItem.hpp"
    "IssuerItem.cpp"
    "IssuerItem.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/AccountSummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/AccountSummaryItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ui/IssuerItem.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/ui
    COMPONENT dev
  )
endif()
