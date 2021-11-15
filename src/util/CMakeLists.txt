# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-util OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/util/Log.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/util/LogMacros.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/util/Exclusive.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/util/Shared.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/util/Types.hpp"
  "Allocator.hpp"
  "AsyncValue.hpp"
  "Backoff.hpp"
  "Blank.hpp"
  "Bytes.cpp"
  "Container.hpp"
  "Exclusive.tpp"
  "Gatekeeper.cpp"
  "Gatekeeper.hpp"
  "HDIndex.hpp"
  "JobCounter.cpp"
  "JobCounter.hpp"
  "Latest.hpp"
  "Log.cpp"
  "Log.hpp"
  "Options.cpp"
  "Options.hpp"
  "Polarity.hpp"
  "Random.cpp"
  "Random.hpp"
  "ScopeGuard.cpp"
  "ScopeGuard.hpp"
  "Signals.cpp"
  "Sodium.cpp"
  "Sodium.hpp"
  "Thread.cpp"
  "Thread.hpp"
  "Time.cpp"
  "Work.hpp"
)
target_include_directories(
  opentxs-util SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Bytes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Log.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Numbers.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Options.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Pimpl.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/SharedPimpl.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Signals.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Time.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/WorkType.hpp"
)

target_link_libraries(
  opentxs-util
  PRIVATE
    opentxs::messages
    Boost::iostreams
    Boost::headers
    Boost::filesystem
    unofficial-sodium::sodium
)
target_link_libraries(
  opentxs
  PUBLIC
    Boost::filesystem
    Boost::iostreams
    Boost::program_options
    unofficial-sodium::sodium
)

if(LMDB_EXPORT)
  target_sources(
    opentxs-util
    PRIVATE
      "LMDB.cpp"
      "LMDB.hpp"
      "MappedFileStorage.cpp"
      "MappedFileStorage.hpp"
  )
  target_link_libraries(opentxs-util PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
endif()

if(UNIX
   AND (NOT APPLE)
   AND (NOT ANDROID)
   AND (NOT OT_VALGRIND)
   AND ("${CMAKE_SIZEOF_VOID_P}"
        EQUAL
        "8")
)
  target_compile_definitions(
    opentxs-util PRIVATE OS_SUPPORTS_LARGE_SPARSE_FILES=1
                         OS_HAS_MEDIOCRE_SPARSE_FILE_SUPPORT=1
  )
else()
  target_compile_definitions(
    opentxs-util PRIVATE OS_SUPPORTS_LARGE_SPARSE_FILES=0
  )

  if("${CMAKE_SIZEOF_VOID_P}"
     EQUAL
     "8"
  )
    target_compile_definitions(
      opentxs-util PRIVATE OS_HAS_MEDIOCRE_SPARSE_FILE_SUPPORT=1
    )
  else()
    target_compile_definitions(
      opentxs-util PRIVATE OS_HAS_MEDIOCRE_SPARSE_FILE_SUPPORT=0
    )
  endif()
endif()

if(WIN32 AND OT_STATIC_DEPENDENCIES)
  target_compile_definitions(
    opentxs-util PRIVATE SODIUM_STATIC=1 SODIUM_EXPORT=
  )
endif()

target_sources(opentxs-util PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-util>)
add_dependencies(opentxs-util generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util
    COMPONENT dev
  )
endif()
