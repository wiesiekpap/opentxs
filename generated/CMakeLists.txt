# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

configure_file(
  "Version.hpp.in"
  "${opentxs_BINARY_DIR}/include/opentxs/Version.hpp"
  @ONLY
)

if(OT_QT_EXPORT)
  file(
    READ
    "opentxs.hpp.qt.in"
    OT_INCLUDE_QT
  )
else()
  set(OT_INCLUDE_QT "")
endif()

configure_file(
  "opentxs.hpp.in"
  "${opentxs_BINARY_DIR}/include/opentxs/opentxs.hpp"
  @ONLY
)

if(OPENTXS_STANDALONE)
  install(
    FILES "${opentxs_BINARY_DIR}/include/opentxs/Version.hpp"
          "${opentxs_BINARY_DIR}/include/opentxs/opentxs.hpp"
    DESTINATION include/opentxs/
    COMPONENT dev
  )
endif()
