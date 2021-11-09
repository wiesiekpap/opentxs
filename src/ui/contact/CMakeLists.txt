# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Contact.cpp"
    "Contact.hpp"
    "ContactItem.cpp"
    "ContactItem.hpp"
    "ContactSection.cpp"
    "ContactSection.hpp"
    "ContactSubsection.cpp"
    "ContactSubsection.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactSubsection.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
