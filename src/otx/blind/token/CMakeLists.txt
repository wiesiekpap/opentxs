# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/blind/Token.hpp"
    "Token.cpp"
    "Token.hpp"
    "Imp.cpp"
    "Imp.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/blind/Token.hpp"
)

if(NOT CASH_LUCRE_EXPORT)
  target_sources(opentxs-common PRIVATE "no-lucre.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/otx/blind
    COMPONENT dev
  )
endif()
