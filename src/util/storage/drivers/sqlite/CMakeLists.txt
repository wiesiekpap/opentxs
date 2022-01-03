# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(SQLITE_EXPORT)
  target_sources(opentxs-common PRIVATE "Sqlite3.cpp" "Sqlite3.hpp")
  target_link_libraries(opentxs-common PRIVATE SQLite::SQLite3)
  target_link_libraries(opentxs PUBLIC SQLite::SQLite3)
else()
  target_sources(opentxs-common PRIVATE "null.cpp")
endif()
