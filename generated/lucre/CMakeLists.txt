# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_LUCRE_DEBUG)
  set(OT_LUCRE_ENABLED "true")
else()
  set(OT_LUCRE_ENABLED "false")
endif()

if(WIN32)
  set(OT_LUCRE_INIT "log_to_file")
else()
  set(OT_LUCRE_INIT "log_to_screen")
endif()

configure_file(
  "LucreConfig.cpp.in"
  "${opentxs_BINARY_DIR}/src/otx/blind/lucre/LucreConfig.cpp"
  @ONLY
)
