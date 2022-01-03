# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
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
