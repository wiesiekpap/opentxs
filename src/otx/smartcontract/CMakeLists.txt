# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-otx-smartcontract OBJECT
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTAgent.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTBylaw.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTClause.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTParty.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTPartyAccount.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTScript.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTScriptChai.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTSmartContract.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTStash.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTStashItem.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/otx/smartcontract/OTVariable.hpp"
  "OTAgent.cpp"
  "OTBylaw.cpp"
  "OTClause.cpp"
  "OTParty.cpp"
  "OTPartyAccount.cpp"
  "OTScript.cpp"
  "OTScriptChai.cpp"
  "OTSmartContract.cpp"
  "OTStash.cpp"
  "OTStashItem.cpp"
  "OTVariable.cpp"
)
target_link_libraries(opentxs-otx-smartcontract PRIVATE opentxs::messages)
target_include_directories(
  opentxs-otx-smartcontract SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/ChaiScript/include"
          "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-otx-smartcontract>)
add_dependencies(opentxs-otx-smartcontract generated_code)
