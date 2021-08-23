# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/DisplayScale.hpp"
)
qt5_wrap_cpp(MOC_SRCS ${cxx-install-headers})
add_library(
  opentxs-ui-displayscale OBJECT
  "DisplayScale.cpp"
  ${MOC_SRCS}
  ${cxx-install-headers}
)
target_link_libraries(
  opentxs-ui-displayscale PRIVATE Qt5::Core opentxs::messages
)
target_link_libraries(opentxs PRIVATE Qt5::Core)
target_include_directories(
  opentxs-ui-displayscale SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-ui-displayscale>)
add_dependencies(opentxs-ui-displayscale generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui/qt
    COMPONENT dev
  )
endif()
