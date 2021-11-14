# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "Contact.cpp"
    "ContactData.cpp"
    "ContactGroup.cpp"
    "ContactItem.cpp"
    "ContactSection.cpp"
    "Internal.cpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactData.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactGroup.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/Attribute.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/ClaimType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/SectionType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/contact/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/contact
    COMPONENT dev
  )
endif()
