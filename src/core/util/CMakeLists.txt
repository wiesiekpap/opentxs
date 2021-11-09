# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-common PRIVATE "Common.cpp" "Tag.cpp")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Common.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Tag.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/util
    COMPONENT dev
  )
endif()
