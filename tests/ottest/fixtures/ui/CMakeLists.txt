# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(ottest PRIVATE "Helpers.hpp" "Helpers.cpp")

if(OT_QT_EXPORT)
  target_sources(ottest PRIVATE "Helpers-qt.cpp")
  target_link_libraries(ottest PRIVATE Qt::Gui)
else()
  target_sources(ottest PRIVATE "Helpers-noqt.cpp")
endif()
