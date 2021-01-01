# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
project(opentxs)
list(APPEND CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}")

if("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
  set(OPENTXS_STANDALONE ON)
  message(STATUS "Configuring Open-Transactions as a library")
else()
  set(OPENTXS_STANDALONE OFF)
  message(STATUS "Configuring Open-Transactions as a subproject")
endif()

# -----------------------------------------------------------------------------
# Setup

include(open-transactions-common)

set_version_from_git()
set_build_type()
set_out_of_source_build()
update_git_submodules()
set_rpath()

# -----------------------------------------------------------------------------
# Set option defaults

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPENTXS_PEDANTIC_DEFAULT ON)
  set(OPENTXS_BUILD_TESTS_DEFAULT ON)
  set(CMAKE_VERBOSE_MAKEFILE ON)
else()
  set(OPENTXS_PEDANTIC_DEFAULT OFF)
  set(OPENTXS_BUILD_TESTS_DEFAULT OFF)
endif()

if(WIN32)
  set(OT_USE_VCPKG_TARGETS_DEFAULT ON)
  set(OT_WITH_BLOCKCHAIN_DEFAULT OFF)
  set(OT_SCRIPT_USING_CHAI_DEFAULT OFF)
  set(OT_ENABLE_RPC_DEFAULT OFF)
else()
  set(OT_USE_VCPKG_TARGETS_DEFAULT OFF)
  set(OT_WITH_BLOCKCHAIN_DEFAULT ON)
  set(OT_SCRIPT_USING_CHAI_DEFAULT ON)
  set(OT_ENABLE_RPC_DEFAULT ON)
endif()
# -----------------------------------------------------------------------------
# Options

option(
  OPENTXS_BUILD_TESTS "Build the unit tests." ${OPENTXS_BUILD_TESTS_DEFAULT}
)
option(
  OPENTXS_PEDANTIC_BUILD "Treat compiler warnings as errors."
  ${OPENTXS_PEDANTIC_DEFAULT}
)
option(OT_VALGRIND "Use Valgrind annotations." OFF)
option(OT_DHT "Enable OpenDHT support" OFF)
option(OT_STORAGE_FS "Enable filesystem backend for storage" OFF)
option(OT_STORAGE_SQLITE "Enable sqlite backend for storage" OFF)
option(OT_STORAGE_LMDB "Enable LMDB backend for storage" ON)
option(OT_CRYPTO_SUPPORTED_KEY_ED25519 "Enable ed25519 key support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_RSA "Enable RSA key support" OFF)
option(OT_CRYPTO_SUPPORTED_KEY_SECP256K1 "Enable secp256k1 key support" ON)
option(OT_CRYPTO_USING_LIBSECP256K1 "Use libsecp256k1 crypto library" ON)
option(
  OT_WITH_BLOCKCHAIN "Enable blockchain client support"
  ${OT_WITH_BLOCKCHAIN_DEFAULT}
)
option(OT_CASH_USING_LUCRE "Enable Lucre support" OFF)
option(OT_LUCRE_DEBUG "Output Lucre debug info" OFF)
option(OT_SCRIPT_USING_CHAI "Use chaiscript" ${OT_SCRIPT_USING_CHAI_DEFAULT})
option(OT_WITH_QT "Enable Qt model support for ui classes" OFF)
option(OT_WITH_QML "Enable Qt QML model support for ui classes" OFF)
option(OT_WITH_JAVA "Build with Java binding" OFF)
option(OT_OPENSSL_FLAVOR_LIBRESSL "Assume OpenSSL library is LibreSSL" OFF)
option(
  OT_USE_VCPKG_TARGETS "Assume dependencies are managed by vcpkg"
  ${OT_USE_VCPKG_TARGETS_DEFAULT}
)
option(
  OT_STATIC_DEPENDENCIES "Link against static version of dependencies"
  ${OT_USE_VCPKG_TARGETS}
)

if(OT_CRYPTO_SUPPORTED_KEY_RSA
   OR OT_CASH_USING_LUCRE
   OR OT_WITH_BLOCKCHAIN
)
  set(OT_CRYPTO_USING_OPENSSL_DEFAULT ON)
else()
  set(OT_CRYPTO_USING_OPENSSL_DEFAULT OFF)
endif()

option(
  OT_CRYPTO_USING_OPENSSL "Use OpenSSL crypto library"
  ${OT_CRYPTO_USING_OPENSSL_DEFAULT}
)

if(NOT OT_WITH_QT)
  if(OT_WITH_QML)
    message(WARNING "QML support requires Qt")
  endif()

  set(OT_WITH_QML OFF)
endif()

if(ANDROID OR OT_USE_VCPKG_TARGETS)
  set(OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT ON)
else()
  set(OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT OFF)
endif()

option(
  OT_INSTALL_LIBRARY_DEPENDENCIES
  "Install dependent libraries along with libopentxs"
  ${OT_INSTALL_LIBRARY_DEPENDENCIES_DEFAULT}
)
option(OT_ENABLE_RPC "Build RPC server" ${OT_ENABLE_RPC_DEFAULT})

# -----------------------------------------------------------------------------
# Set compiler options

set_compiler_flags(17 ${OPENTXS_PEDANTIC_BUILD})

# temporarily disabled warnings
if(MSVC)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /wd4244 /wd4251 /wd4267 /wd4275 /wd4996"
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
else()
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-format-nonliteral -Wno-noexcept -Wno-redundant-move -Wno-shadow -Wno-sign-conversion -Wno-switch-enum -Wno-undef -Wno-unused-parameter"
  )
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-c++2a-compat -Wno-c++20-compat -Wno-comma -Wno-disabled-macro-expansion -Wno-documentation -Wno-documentation-unknown-command -Wno-extra-semi-stmt -Wno-implicit-int-conversion -Wno-redundant-parens -Wno-reserved-id-macro -Wno-shorten-64-to-32 -Wno-unused-template"
  )
endif()

# Protobuf generated code will generate this warning
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=extra-semi")
endif()

# -----------------------------------------------------------------------------
# Print system information and build options

print_build_details(OPENTXS_PEDANTIC_BUILD OPENTXS_BUILD_TESTS)

message(STATUS "Valgrind integration:   ${OT_VALGRIND}")

message(STATUS "Network plugins------------------------------")
message(STATUS "DHT:                    ${OT_DHT}")

message(STATUS "Storage backends-----------------------------")
message(STATUS "filesystem:             ${OT_STORAGE_FS}")
message(STATUS "sqlite                  ${OT_STORAGE_SQLITE}")
message(STATUS "LMDB                    ${OT_STORAGE_LMDB}")

message(STATUS "Key algorithms-------------------------------")
message(STATUS "ed25519:                ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
message(STATUS "RSA:                    ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
message(STATUS "secp256k1               ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}")

message(STATUS "Crypto library providers---------------------")
message(STATUS "OpenSSL:                ${OT_CRYPTO_USING_OPENSSL}")
message(STATUS "libsecp256k1:           ${OT_CRYPTO_USING_LIBSECP256K1}")

message(STATUS "Blockchain-----------------------------------")
message(STATUS "Blockchain client:      ${OT_WITH_BLOCKCHAIN}")

message(STATUS "Cash library providers-----------------------")
message(STATUS "Lucre:                  ${OT_CASH_USING_LUCRE}")
message(STATUS "Lucre debug info:       ${OT_LUCRE_DEBUG}")

message(STATUS "Script engines-------------------------------")
message(STATUS "Chai:                   ${OT_SCRIPT_USING_CHAI}")

message(STATUS "UI-------------------------------------------")
message(STATUS "Qt:                     ${OT_WITH_QT}")
message(STATUS "QML:                    ${OT_WITH_QML}")

message(STATUS "Bindings ------------------------------------")
message(STATUS "Java binding:           ${OT_WITH_JAVA}")

message(STATUS "Extra ---------------------------------------")
message(STATUS "RPC server:             ${OT_ENABLE_RPC}")

# -----------------------------------------------------------------------------
# Dependencies

if(OT_STATIC_DEPENDENCIES)
  set(Protobuf_USE_STATIC_LIBS ON)
endif()

find_system_libraries()

if(OPENTXS_BUILD_TESTS)
  if(OT_USE_VCPKG_TARGETS)
    find_package(GTest CONFIG REQUIRED)
  else()
    find_package(GTest REQUIRED)
  endif()
  enable_testing()
endif()

find_package(unofficial-sodium REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(
  Boost 1.73.0 REQUIRED
  COMPONENTS system filesystem iostreams
  OPTIONAL_COMPONENTS stacktrace_basic
)

if(OT_USE_VCPKG_TARGETS)
  find_package(ZeroMQ REQUIRED)
else()
  find_package(unofficial-zeromq REQUIRED)
endif()

if(OT_STATIC_DEPENDENCIES)
  set(OT_ZMQ_TARGET "libzmq-static")
else()
  set(OT_ZMQ_TARGET "libzmq")
endif()

if(OT_WITH_BLOCKCHAIN)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    if(ANDROID
       OR APPLE
       OR WIN32
    )
      set(OPENTXS_BLOCK_STORAGE_ENABLED 0)
      set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 0)
    else()
      set(OPENTXS_BLOCK_STORAGE_ENABLED 1)
      set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 1)
    endif()
  else()
    set(OPENTXS_BLOCK_STORAGE_ENABLED 0)
    set(OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY 0)
  endif()

  find_package(Boost REQUIRED thread)
endif()

if(OT_DHT)
  find_package(opendht 1.10 REQUIRED)
  find_package(GnuTLS REQUIRED)
endif()

if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()

if(OT_STORAGE_LMDB)
  find_package(lmdb REQUIRED)
endif()

if(OT_CRYPTO_USING_OPENSSL)
  if(OT_STATIC_DEPENDENCIES)
    set(OPENSSL_USE_STATIC_LIBS ON)
  endif()

  if(CMAKE_MSVC_RUNTIME_LIBRARY STREQUAL "MultiThreaded")
    set(OPENSSL_MSVC_STATIC_RT ON)
  endif()

  find_package(OpenSSL 1.0.1 REQUIRED)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  find_package(unofficial-secp256k1 REQUIRED)
endif()

if(OT_WITH_QT)
  find_package(
    Qt5
    COMPONENTS Core Gui Widgets
    REQUIRED
  )

  if(OT_WITH_QML)
    find_package(
      Qt5
      COMPONENTS Qml
      REQUIRED
    )
  endif()

  set_target_properties(
    Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE"
  )
  list(
    GET
    Qt5Core_INCLUDE_DIRS
    0
    QT_INCLUDE_DIR
  )
  include_directories(SYSTEM "${QT_INCLUDE_DIR}")
endif()

# -----------------------------------------------------------------------------
# Source Definitions

set_common_defines()
hide_symbols("OPENTXS_EXPORT")

if(APPLE)
  add_definitions(-DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED)
endif()

if(OT_VALGRIND)
  add_definitions(-DOT_VALGRIND=1)
else()
  add_definitions(-DOT_VALGRIND=0)
endif()

# Network

if(OT_DHT)
  set(DHT_EXPORT 1)
else()
  set(DHT_EXPORT 0)
endif()

# Storage backends

if(OT_STORAGE_FS)
  set(FS_EXPORT 1)
else()
  set(FS_EXPORT 0)
endif()

if(OT_STORAGE_SQLITE)
  set(SQLITE_EXPORT 1)
else()
  set(SQLITE_EXPORT 0)
endif()

if(OT_STORAGE_LMDB)
  set(LMDB_EXPORT 1)
else()
  set(LMDB_EXPORT 0)
endif()

if((NOT OT_STORAGE_FS)
   AND (NOT OT_STORAGE_SQLITE)
   AND (NOT OT_STORAGE_LMDB)
)
  message(FATAL_ERROR "At least one storage backend must be defined.")
endif()

# Key types

if(NOT OT_CRYPTO_SUPPORTED_KEY_ED25519)
  set(ED25519_EXPORT 0)
  if(NOT OT_CRYPTO_SUPPORTED_KEY_RSA)
    if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
      message(FATAL_ERROR "At least one type of key must be enabled")
    endif()
  endif()
else()
  set(ED25519_EXPORT 1)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_RSA)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "RSA support requires OpenSSL")
  endif()
  set(RSA_EXPORT 1)
else()
  set(RSA_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  if(NOT OT_CRYPTO_USING_LIBSECP256K1)
    message(FATAL_ERROR "secp256k1 key support requires libsecp256k1")
  endif()
  set(SECP256K1_EXPORT 1)
else()
  set(SECP256K1_EXPORT 0)
endif()

# Crypto libraries

if(OT_CRYPTO_USING_OPENSSL)
  set(OPENSSL_EXPORT 1)
else()
  set(OPENSSL_EXPORT 0)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(LIBSECP256K1_EXPORT 1)
  else()
    set(LIBSECP256K1_EXPORT 0)
    message(WARNING "Libsecp256k1 is pointless without secp256k1 keys.")
  endif()
else()
  set(LIBSECP256K1_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  set(BIP32_EXPORT 1)
else()
  set(BIP32_EXPORT 0)
endif()

if(OT_WITH_BLOCKCHAIN)
  if(NOT OT_STORAGE_LMDB)
    message(FATAL_ERROR "Blockchain support requires LMDB")
  endif()

  if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "Blockchain support requires secp256k1 key support")
  endif()

  if(NOT BIP32_EXPORT)
    message(FATAL_ERROR "Blockchain support requires BIP32 support")
  endif()

  set(OT_BLOCKCHAIN_EXPORT 1)
else()
  set(OT_BLOCKCHAIN_EXPORT 0)
endif()

if(OT_CASH_USING_LUCRE)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "Lucre support requires OpenSSL")
  endif()

  set(CASH_LUCRE_EXPORT 1)
  set(OT_CASH_EXPORT 1)
else()
  set(CASH_LUCRE_EXPORT 0)
  set(OT_CASH_EXPORT 0)
endif()

if(FALSE)
  set(CASH_MM_EXPORT 1)
  set(OT_CASH_EXPORT 1)
else()
  set(CASH_MM_EXPORT 0)
endif()

# Script engines
if(OT_SCRIPT_USING_CHAI)
  set(SCRIPT_CHAI_EXPORT 1)
else()
  set(SCRIPT_CHAI_EXPORT 0)
endif()

if(OT_WITH_QT)
  set(OT_QT_EXPORT 1)
else()
  set(OT_QT_EXPORT 0)
endif()

if(OT_WITH_QML)
  set(OT_QML_EXPORT 1)
else()
  set(OT_QML_EXPORT 0)
endif()

if(OT_WITH_JAVA)
  set(OT_SWIG ON)
else()
  set(OT_SWIG OFF)
endif()

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# -----------------------------------------------------------------------------
# Set in-source Version
configure_file(
  "Version.hpp.in" "${opentxs_BINARY_DIR}/include/opentxs/Version.hpp" @ONLY
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${opentxs_BINARY_DIR}/include/opentxs/Version.hpp
    DESTINATION include/opentxs/
    COMPONENT dev
  )
endif()

# -----------------------------------------------------------------------------
# Build source

include(opentxs-link)

include_directories(
  "${opentxs_BINARY_DIR}/include"
  "${opentxs_BINARY_DIR}/serialization"
  "${opentxs_BINARY_DIR}/src"
  "${opentxs_SOURCE_DIR}/include"
  "${opentxs_SOURCE_DIR}/src"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(deps)
add_subdirectory(serialization)
add_subdirectory(src)

if(OT_SWIG)
  add_subdirectory(wrappers)
endif()

if(OPENTXS_BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Package

if(OPENTXS_STANDALONE)
  generate_cmake_files()
  generate_pkgconfig("-lopentxs")
  generate_uninstall_target()
endif()
