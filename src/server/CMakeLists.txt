# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-server OBJECT
  "ConfigLoader.cpp"
  "ConfigLoader.hpp"
  "Macros.hpp"
  "MainFile.cpp"
  "MainFile.hpp"
  "MessageProcessor.cpp"
  "MessageProcessor.hpp"
  "Notary.cpp"
  "Notary.hpp"
  "PayDividendVisitor.cpp"
  "PayDividendVisitor.hpp"
  "ReplyMessage.cpp"
  "ReplyMessage.hpp"
  "Server.cpp"
  "Server.hpp"
  "ServerSettings.cpp"
  "ServerSettings.hpp"
  "Transactor.cpp"
  "Transactor.hpp"
  "UserCommandProcessor.cpp"
  "UserCommandProcessor.hpp"
)
target_link_libraries(opentxs-server PRIVATE opentxs::messages)
target_include_directories(
  opentxs-server SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-server>)
add_dependencies(opentxs-server generated_code)
