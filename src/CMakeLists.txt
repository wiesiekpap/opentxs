# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(api)
add_subdirectory(blind)
add_subdirectory(blockchain)
add_subdirectory(client)
add_subdirectory(contact)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(display)
add_subdirectory(ext)
add_subdirectory(identity)
add_subdirectory(iterator)
add_subdirectory(license)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(protobuf)
add_subdirectory(rpc)
add_subdirectory(server)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(util)

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${opentxs_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)

set(cxx-install-headers
    "${opentxs_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Bytes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Exclusive.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/OT.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Pimpl.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Shared.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/SharedPimpl.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Types.hpp"
)
target_sources(
  opentxs
  PRIVATE
    "0_stdafx.hpp"
    "1_Internal.hpp"
    "2_Factory.hpp"
    "Bytes.cpp"
    "Exclusive.tpp"
    "OT.cpp"
    "Proto.cpp"
    "Proto.hpp"
    "Proto.tpp"
)
set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION 1
    POSITION_INDEPENDENT_CODE 1
)

if(CMAKE_DL_LIBS)
  target_link_libraries(opentxs PUBLIC ${CMAKE_DL_LIBS})
endif()

if(LIB_RT)
  target_link_libraries(opentxs PUBLIC ${LIB_RT})
endif()

if(LIB_ANDROID_LOG)
  target_link_libraries(opentxs PUBLIC ${LIB_ANDROID_LOG})
endif()

if(Boost_stacktrace_basic_FOUND)
  target_link_libraries(opentxs PUBLIC Boost::stacktrace_basic)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs
    COMPONENT dev
  )
endif()

if(OPENTXS_STANDALONE OR OT_INSTALL_LIBRARY_DEPENDENCIES)
  install(
    TARGETS opentxs
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT opentxs-targets
    COMPONENT main
  )
endif()

if(OT_INSTALL_LIBRARY_DEPENDENCIES)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
  get_target_property(
    Boost_filesystem_LIBRARY
    Boost::filesystem
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_iostreams_LIBRARY
    Boost::iostreams
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_system_LIBRARY
    Boost::system
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_thread_LIBRARY
    Boost::thread
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Protobuf_LITE_LIBRARY
    protobuf::libprotobuf-lite
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    ZLIB_LIBRARY
    ZLIB::ZLIB
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )

  if(OT_USE_VCPKG_TARGETS)
    get_target_property(
      ZMQ_LIBRARY_PATH
      "${OT_ZMQ_TARGET}"
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
  else()
    set(ZMQ_LIBRARY_PATH "${ZMQ_LIBRARY}")
  endif()

  if(OPENTXS_BUILD_TESTS)
    install(
      FILES "${GTEST_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(OPENTXS_BUILD_TESTS)

  install(
    FILES
      "${Boost_filesystem_LIBRARY}"
      "${Boost_iostreams_LIBRARY}"
      "${Boost_system_LIBRARY}"
      "${Boost_thread_LIBRARY}"
      "${Protobuf_LITE_LIBRARY}"
      "${SODIUM_LIBRARY}"
      "${ZLIB_LIBRARY}"
      "${ZMQ_LIBRARY_PATH}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT main
  )

  if(Boost_stacktrace_basic_FOUND)
    get_target_property(
      Boost_stacktrace_basic_LIBRARY
      Boost::stacktrace_basic
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    install(
      FILES "${Boost_stacktrace_basic_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(DHT_EXPORT)
    message(WARNING "Installing OpenDHT dependencies not supported yet")
  endif(DHT_EXPORT)

  if(SQLITE_EXPORT)
    get_target_property(
      SQLITE3_LIBRARY
      SQLite::SQLite3
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )

    install(
      FILES "${SQLITE3_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(SQLITE_EXPORT)

  if(LMDB_EXPORT)
    install(
      FILES "${LMDB_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(OPENSSL_EXPORT)
    install(
      FILES "${OPENSSL_CRYPTO_LIBRARY}" "${OPENSSL_SSL_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(LIBSECP256K1_EXPORT AND NOT OT_BUNDLED_SECP256K1)
    install(
      FILES "${SECP256K1_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()
endif()
