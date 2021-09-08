# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-contact OBJECT
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
target_link_libraries(opentxs-ui-contact PRIVATE opentxs::messages)
target_sources(opentxs-ui-contact PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-contact>)
add_dependencies(opentxs-ui-contact generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
