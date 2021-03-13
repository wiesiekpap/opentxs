# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Nym.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/Server.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/identifier/UnitDefinition.hpp"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/identifier
    COMPONENT dev
  )
endif()
