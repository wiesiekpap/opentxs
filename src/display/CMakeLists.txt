# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-display OBJECT
  "Definition.cpp"
  "Definition.hpp"
  "Definition_imp.hpp"
  "Scale.cpp"
  "Scale.hpp"
  "Scale_imp.hpp"
)
target_link_libraries(opentxs-display PRIVATE Boost::headers opentxs::messages)
target_sources(opentxs-display PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-display>)
add_dependencies(opentxs-display generated_code)
