# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources "SeedValidator.cpp")
set(ot-qobject-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/qt/SeedValidator.hpp"
)
set(cxx-install-headers ${ot-qobject-install-headers})
set(cxx-headers ${cxx-install-headers} "")

qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})

add_library(
  opentxs-ui-seedvalidator OBJECT ${cxx-sources} ${cxx-headers} ${MOC_SRCS}
)
target_link_libraries(
  opentxs-ui-seedvalidator
  PUBLIC Qt5::Core Qt5::Gui
  PRIVATE opentxs::messages
)
target_include_directories(
  opentxs-ui-seedvalidator SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

add_dependencies(opentxs-ui-seedvalidator generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui/qt
    COMPONENT dev
  )
endif()
