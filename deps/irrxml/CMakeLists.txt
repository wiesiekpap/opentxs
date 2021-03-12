# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(irrxml OBJECT "irrXML.cpp")
set(cxx-install-headers
    "CXMLReaderImpl.hpp"
    "fast_atof.hpp"
    "heapsort.hpp"
    "irrArray.hpp"
    "irrString.hpp"
    "irrTypes.hpp"
    "irrXML.hpp"
)

if(NOT MSVC)
  target_compile_options(
    irrxml
    PRIVATE
      -Wno-cast-align
      -Wno-cast-qual
      -Wno-conversion
      -Wno-effc++
      -Wno-extra-semi
      -Wno-implicit-int-conversion
      -Wno-old-style-cast
      -Wno-sign-conversion
      -Wno-zero-as-null-pointer-constant
  )
endif()

if(${CMAKE_CXX_COMPILER_ID}
   MATCHES
   Clang
)
  target_compile_options(
    irrxml
    PRIVATE
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-reserved-id-macro
      -Wno-shorten-64-to-32
      -Wno-suggest-destructor-override
      -Wno-suggest-override
  )
endif()

target_sources(irrxml PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:irrxml>)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/irrxml
    COMPONENT dev
  )
endif()
