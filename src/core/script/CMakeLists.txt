# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-core-script OBJECT
  "OTAgent.cpp"
  "OTBylaw.cpp"
  "OTClause.cpp"
  "OTParty.cpp"
  "OTPartyAccount.cpp"
  "OTScript.cpp"
  "OTScriptChai.cpp"
  "OTScriptable.cpp"
  "OTSmartContract.cpp"
  "OTStash.cpp"
  "OTStashItem.cpp"
  "OTVariable.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTAgent.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTBylaw.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTClause.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTParty.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTPartyAccount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScript.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScriptable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScriptChai.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTSmartContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTStash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTStashItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTVariable.hpp"
)
target_link_libraries(opentxs-core-script PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-script SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/ChaiScript/include"
          "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-core-script PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-script>)
add_dependencies(opentxs-core-script generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/script
    COMPONENT dev
  )
endif()
