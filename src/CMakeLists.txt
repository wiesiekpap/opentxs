# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_custom_target(generated_code)

add_subdirectory(api)

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
  set(object-deps $<TARGET_OBJECTS:opentxs-blind>)
else()
  set(object-deps "")
endif()

add_subdirectory(client)
add_subdirectory(contact)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(display)
add_subdirectory(ext)
add_subdirectory(identity)
add_subdirectory(iterator)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(protobuf)

if(OT_ENABLE_RPC)
  add_subdirectory(rpc)
endif()

add_subdirectory(server)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(util)

if(LIBSECP256K1_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-crypto-library-libsecp256k1>)
endif()

if(OPENSSL_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-crypto-library-openssl>)
endif()

add_subdirectory(blockchain)
list(APPEND object-deps $<TARGET_OBJECTS:opentxs-blockchain>)

if(OT_BLOCKCHAIN_EXPORT)
  list(
    APPEND
    object-deps
    $<TARGET_OBJECTS:opentxs-api-client-blockchain-database>
    $<TARGET_OBJECTS:opentxs-blockchain-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-block-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-block-pkt>
    $<TARGET_OBJECTS:opentxs-blockchain-block>
    $<TARGET_OBJECTS:opentxs-blockchain-client-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-client-wallet>
    $<TARGET_OBJECTS:opentxs-blockchain-client>
    $<TARGET_OBJECTS:opentxs-blockchain-database>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin-message>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p>
    $<TARGET_OBJECTS:opentxs-ui-blockchainselection>
  )
endif()

if(CASH_LUCRE_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:lucre>)
endif()

list(
  APPEND
  object-deps
  $<TARGET_OBJECTS:base64>
  $<TARGET_OBJECTS:external-hashes>
  $<TARGET_OBJECTS:irrxml>
  $<TARGET_OBJECTS:messages>
  $<TARGET_OBJECTS:opentxs-api-client-blockchain>
  $<TARGET_OBJECTS:opentxs-api-client>
  $<TARGET_OBJECTS:opentxs-api-crypto-base58>
  $<TARGET_OBJECTS:opentxs-api-crypto>
  $<TARGET_OBJECTS:opentxs-api-network>
  $<TARGET_OBJECTS:opentxs-api-server>
  $<TARGET_OBJECTS:opentxs-api-storage>
  $<TARGET_OBJECTS:opentxs-api>
  $<TARGET_OBJECTS:opentxs-client>
  $<TARGET_OBJECTS:opentxs-contact>
  $<TARGET_OBJECTS:opentxs-core-contract-basket>
  $<TARGET_OBJECTS:opentxs-core-contract-peer>
  $<TARGET_OBJECTS:opentxs-core-contract>
  $<TARGET_OBJECTS:opentxs-core-crypto>
  $<TARGET_OBJECTS:opentxs-core-transaction>
  $<TARGET_OBJECTS:opentxs-core-util>
  $<TARGET_OBJECTS:opentxs-core>
  $<TARGET_OBJECTS:opentxs-cron>
  $<TARGET_OBJECTS:opentxs-crypto-key>
  $<TARGET_OBJECTS:opentxs-crypto-library>
  $<TARGET_OBJECTS:opentxs-crypto>
  $<TARGET_OBJECTS:opentxs-display>
  $<TARGET_OBJECTS:opentxs-ext>
  $<TARGET_OBJECTS:opentxs-identity-credential>
  $<TARGET_OBJECTS:opentxs-identity-wot-verification>
  $<TARGET_OBJECTS:opentxs-identity>
  $<TARGET_OBJECTS:opentxs-iterator>
  $<TARGET_OBJECTS:opentxs-network-zeromq-curve>
  $<TARGET_OBJECTS:opentxs-network-zeromq-socket>
  $<TARGET_OBJECTS:opentxs-network-zeromq-zap>
  $<TARGET_OBJECTS:opentxs-network-zeromq>
  $<TARGET_OBJECTS:opentxs-network>
  $<TARGET_OBJECTS:opentxs-otx-client>
  $<TARGET_OBJECTS:opentxs-otx-consensus>
  $<TARGET_OBJECTS:opentxs-otx>
  $<TARGET_OBJECTS:opentxs-recurring>
  $<TARGET_OBJECTS:opentxs-script>
  $<TARGET_OBJECTS:opentxs-server>
  $<TARGET_OBJECTS:opentxs-storage-drivers>
  $<TARGET_OBJECTS:opentxs-storage-tree>
  $<TARGET_OBJECTS:opentxs-storage>
  $<TARGET_OBJECTS:opentxs-trade>
  $<TARGET_OBJECTS:opentxs-ui-accountactivity>
  $<TARGET_OBJECTS:opentxs-ui-accountlist>
  $<TARGET_OBJECTS:opentxs-ui-accountsummary>
  $<TARGET_OBJECTS:opentxs-ui-activitysummary>
  $<TARGET_OBJECTS:opentxs-ui-activitythread>
  $<TARGET_OBJECTS:opentxs-ui-base>
  $<TARGET_OBJECTS:opentxs-ui-contact>
  $<TARGET_OBJECTS:opentxs-ui-contactlist>
  $<TARGET_OBJECTS:opentxs-ui-messagablelist>
  $<TARGET_OBJECTS:opentxs-ui-payablelist>
  $<TARGET_OBJECTS:opentxs-ui-profile>
  $<TARGET_OBJECTS:opentxs-ui-unitlist>
  $<TARGET_OBJECTS:opentxs-util>
  $<TARGET_OBJECTS:otprotob>
  $<TARGET_OBJECTS:proto-verify>
)

if(OT_QT_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-ui-displayscale>)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-ui-seedvalidator>)
endif()

if(OT_ENABLE_RPC)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-rpc>)
endif()

file(
  READ "${CMAKE_CURRENT_SOURCE_DIR}/api/crypto/base58/COPYING"
  BASE58_LICENSE_TEXT
)
file(READ "${opentxs_SOURCE_DIR}/deps/base64/COPYING" BASE64_LICENSE_TEXT)
file(
  READ "${opentxs_SOURCE_DIR}/deps/ChaiScript/LICENSE" CHAISCRIPT_LICENSE_TEXT
)
file(READ "${opentxs_SOURCE_DIR}/deps/irrxml/COPYING" IRRXML_LICENSE_TEXT)
file(READ "${opentxs_SOURCE_DIR}/deps/lucre/LICENCE" LUCRE_LICENSE_TEXT)
file(READ "${opentxs_SOURCE_DIR}/LICENSE" OPENTXS_LICENSE_TEXT)
file(READ "${opentxs_SOURCE_DIR}/CREDITS" OPENTXS_CREDITS_TEXT)
file(READ "${opentxs_SOURCE_DIR}/MPL-2.0" MPL20_LICENSE_TEXT)
file(
  READ "${opentxs_SOURCE_DIR}/serialization/LICENSE.txt" PROTOBUF_LICENSE_TEXT
)
file(
  READ "${opentxs_SOURCE_DIR}/deps/simpleini/LICENCE.txt"
  SIMPLEINI_LICENSE_TEXT
)
file(
  READ "${CMAKE_CURRENT_SOURCE_DIR}/crypto/library/trezor/COPYING"
  TREZOR_LICENSE_TEXT
)

configure_file("License.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/License.cpp" @ONLY)

set(cxx-sources
    "${CMAKE_CURRENT_SOURCE_DIR}/Bytes.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/License.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/OT.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Proto.cpp"
)

set(cxx-install-headers
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Bytes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Exclusive.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Forward.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/OT.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Pimpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.tpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Shared.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/SharedPimpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Types.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/opentxs.hpp"
)

set(cxx-headers
    ${cxx-install-headers}
    "${CMAKE_CURRENT_SOURCE_DIR}/0_stdafx.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/1_Internal.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/2_Factory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exclusive.tpp"
)

add_library(opentxs-common OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-common PRIVATE opentxs::messages)

list(APPEND object-deps $<TARGET_OBJECTS:opentxs-common>)

if(OPENTXS_STANDALONE)
  # library type controlled by BUILD_SHARED_LIBS
  add_library(opentxs ${object-deps})
else()
  add_library(opentxs STATIC ${object-deps})
  target_include_directories(
    opentxs
    INTERFACE
      $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/serialization>
      $<BUILD_INTERFACE:${opentxs_BINARY_DIR}/src>
      $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/deps>
      $<BUILD_INTERFACE:${opentxs_SOURCE_DIR}/include>
  )
endif()

opentxs_link_target("opentxs")
add_library(opentxs::libopentxs ALIAS opentxs)

set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION 1
)

add_dependencies(generated_code otprotob)
add_dependencies(generated_code messages)

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${${PROJECT_NAME}_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)

if(OPENTXS_STANDALONE)
  install(
    TARGETS opentxs
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT opentxs-targets
    COMPONENT main
  )
endif()

if(OT_INSTALL_LIBRARY_DEPENDENCIES AND OPENTXS_STANDALONE)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
  get_target_property(
    Boost_system_LIBRARY Boost::system IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_filesystem_LIBRARY Boost::filesystem
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_iostreams_LIBRARY Boost::iostreams
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Protobuf_LITE_LIBRARY protobuf::libprotobuf-lite
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    ZLIB_LIBRARY ZLIB::ZLIB IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )

  if(OT_USE_VCPKG_TARGETS)
    get_target_property(
      ZMQ_LIBRARY_PATH "${OT_ZMQ_TARGET}"
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
      "${SODIUM_LIBRARY}"
      "${ZMQ_LIBRARY_PATH}"
      "${Protobuf_LITE_LIBRARY}"
      "${ZLIB_LIBRARY}"
      "${Boost_system_LIBRARY}"
      "${Boost_filesystem_LIBRARY}"
      "${Boost_iostreams_LIBRARY}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT main
  )

  if(Boost_stacktrace_basic_FOUND)
    get_target_property(
      Boost_stacktrace_basic_LIBRARY Boost::stacktrace_basic
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    install(
      FILES "${Boost_stacktrace_basic_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(OT_BLOCKCHAIN_EXPORT)
    get_target_property(
      Boost_thread_LIBRARY Boost::thread IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )

    install(
      FILES "${Boost_thread_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(OT_BLOCKCHAIN_EXPORT)

  if(DHT_EXPORT)
    message(WARNING "Installing OpenDHT dependencies not supported yet")
  endif(DHT_EXPORT)

  if(SQLITE_EXPORT)
    get_target_property(
      SQLITE3_LIBRARY SQLite::SQLite3 IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
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

  if(LIBSECP256K1_EXPORT)
    install(
      FILES "${SECP256K1_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()
endif(OT_INSTALL_LIBRARY_DEPENDENCIES AND OPENTXS_STANDALONE)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
          "${${PROJECT_NAME}_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
    DESTINATION include/opentxs
    COMPONENT dev
  )
endif()
