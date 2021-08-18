# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-argon2 OBJECT
  "argon2.c"
  "argon2.h"
  "blake2-impl.h"
  "blake2.h"
  "blake2b.c"
  "core.c"
  "core.h"
  "encoding.c"
  "encoding.h"
)

if(ANDROID)
  target_sources(opentxs-argon2 PRIVATE "blamka-round-ref.h" "ref.c")
else()
  target_sources(opentxs-argon2 PRIVATE "blamka-round-opt.h" "opt.c")
endif()

target_include_directories(
  opentxs-argon2 SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_definitions(opentxs-argon2 PRIVATE "ARGON2_NO_THREADS")
set_target_properties(
  opentxs-argon2 PROPERTIES C_INCLUDE_WHAT_YOU_USE "" C_STANDARD 90
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-argon2>)
