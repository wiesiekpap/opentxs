# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(opentxs_link_target target_name)

  target_link_libraries(
    ${target_name}
    PRIVATE
      Threads::Threads
      ZLIB::ZLIB
      Boost::filesystem
      Boost::iostreams
      Boost::system
      protobuf::libprotobuf-lite
      "${OT_ZMQ_TARGET}"
      unofficial-sodium::sodium
  )

  if(Boost_stacktrace_basic_FOUND)
    target_link_libraries(${target_name} PRIVATE Boost::stacktrace_basic)
  endif()

  if(OT_BLOCKCHAIN_EXPORT)
    target_link_libraries(${target_name} PRIVATE Boost::thread)
  endif()

  if(CMAKE_DL_LIBS)
    target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS})
  endif()

  if(LIB_RT)
    target_link_libraries(${target_name} PRIVATE ${LIB_RT})
  endif()

  if(LIB_ANDROID_LOG)
    target_link_libraries(${target_name} PRIVATE ${LIB_ANDROID_LOG})
  endif()

  if(DHT_EXPORT)
    target_link_libraries(${target_name} PRIVATE opendht ${GNUTLS_LIBRARIES})
  endif()

  if(SQLITE_EXPORT)
    target_link_libraries(${target_name} PRIVATE SQLite::SQLite3)
  endif()

  if(LMDB_EXPORT)
    target_link_libraries(${target_name} PRIVATE lmdb)
  endif()

  if(OPENSSL_EXPORT)
    target_link_libraries(${target_name} PRIVATE OpenSSL::Crypto OpenSSL::SSL)

    if(WIN32 AND OT_STATIC_DEPENDENCIES)
      target_link_libraries(${target_name} PRIVATE CRYPT32.LIB)
    endif()
  endif()

  if(LIBSECP256K1_EXPORT)
    target_link_libraries(${target_name} PRIVATE unofficial::secp256k1)
  endif()

  if(OT_QT_EXPORT)
    target_link_libraries(
      ${target_name} PRIVATE ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES}
    )

    if(OT_QML_EXPORT)
      target_link_libraries(${target_name} PRIVATE ${Qt5Qml_LIBRARIES})
    endif()
  endif()

  target_include_directories(
    ${target_name} SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
  )
  target_include_directories(
    ${target_name} SYSTEM PRIVATE "${Protobuf_INCLUDE_DIRS}"
  )
  set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE 1)
endmacro()
