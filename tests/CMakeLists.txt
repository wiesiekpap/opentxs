# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(
  add_opentx_test_target
  target_name
  cxx-sources
)
  include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
    ${GTEST_INCLUDE_DIRS}
  )

  add_executable(${target_name} ${cxx-sources})

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Helpers.cpp")
    target_sources(
      ${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Helpers.cpp"
    )
  endif()

  target_link_libraries(
    ${target_name} PRIVATE opentxs::libopentxs Boost::program_options
  )

  if(OT_USE_VCPKG_TARGETS)
    target_link_libraries(${target_name} PRIVATE GTest::gtest)
  else()
    target_link_libraries(${target_name} PRIVATE GTest::GTest)
  endif()

  target_include_directories(
    ${target_name} SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
  )
  set_target_properties(
    ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                              ${PROJECT_BINARY_DIR}/tests
  )

  if(${CMAKE_CXX_COMPILER_ID}
     MATCHES
     Clang
  )
    target_compile_options(
      ${target_name} PRIVATE -Wno-suggest-destructor-override
    )
  endif()

  add_test(
    ${target_name}
    ${PROJECT_BINARY_DIR}/tests/${target_name}
    --gtest_output=xml:gtestresults.xml
  )
endfunction()

function(
  add_opentx_test
  target_name
  file_name
)
  set(cxx-sources
      "${PROJECT_SOURCE_DIR}/tests/main.cpp"
      "${file_name}"
      "${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp"
  )

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

function(
  add_opentx_low_level_test
  target_name
  file_name
)
  set(cxx-sources
      "${PROJECT_SOURCE_DIR}/tests/lowlevel.cpp"
      "${file_name}"
      "${PROJECT_SOURCE_DIR}/tests/OTLowLevelTestEnvironment.cpp"
  )

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

add_subdirectory(blockchain)

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
endif()

add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(contact)
add_subdirectory(context)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(otx)

if(SECP256K1_EXPORT AND BIP32_EXPORT)
  add_subdirectory(paymentcode)
endif()

if(OT_ENABLE_RPC)
  add_subdirectory(rpc)
endif()

add_subdirectory(ui)
