# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-ui-profile OBJECT
  "Profile.cpp"
  "Profile.hpp"
  "ProfileItem.cpp"
  "ProfileItem.hpp"
  "ProfileSection.cpp"
  "ProfileSection.hpp"
  "ProfileSubsection.cpp"
  "ProfileSubsection.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Profile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSubsection.hpp"
)
target_link_libraries(opentxs-ui-profile PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-profile SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-ui-profile PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-profile>)
add_dependencies(opentxs-ui-profile generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
