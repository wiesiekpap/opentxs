# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(CASH_LUCRE_EXPORT)
  add_subdirectory(lucre)
endif()
add_subdirectory(mint)
add_subdirectory(purse)
add_subdirectory(token)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/blind/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/blind/Types.hpp"
    "Blind.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/blind/CashType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/blind/PurseType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/blind/TokenState.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/blind/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/otx/blind
    COMPONENT dev
  )
endif()
