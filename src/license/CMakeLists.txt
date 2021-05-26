# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file(
  READ
  "${opentxs_SOURCE_DIR}/src/api/crypto/base58/COPYING"
  BASE58_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/base64/COPYING"
  BASE64_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/ChaiScript/LICENSE"
  CHAISCRIPT_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/irrxml/COPYING"
  IRRXML_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/lucre/LICENCE"
  LUCRE_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/LICENSE"
  OPENTXS_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/CREDITS"
  OPENTXS_CREDITS_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/MPL-2.0"
  MPL20_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/serialization/LICENSE.txt"
  PROTOBUF_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/simpleini/LICENCE.txt"
  SIMPLEINI_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/src/crypto/library/trezor/COPYING"
  TREZOR_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/LICENSE.bech32"
  BECH32_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/deps/secp256k1/COPYING"
  LIBSECP256K1_LICENSE_TEXT
)
configure_file(
  "License.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/License.cpp"
  @ONLY
)

add_library(opentxs-license OBJECT "${CMAKE_CURRENT_BINARY_DIR}/License.cpp")

target_compile_definitions(
  opentxs-license
  PRIVATE
    OT_SCRIPT_CHAI=${SCRIPT_CHAI_EXPORT}
    OT_CASH_USING_LUCRE=${CASH_LUCRE_EXPORT}
    OT_CRYPTO_USING_PACKETCRYPT=${PACKETCRYPT_EXPORT}
    OT_CRYPTO_USING_LIBSECP256K1=${LIBSECP256K1_EXPORT}
)
set_target_properties(
  opentxs-license PROPERTIES C_INCLUDE_WHAT_YOU_USE "" CXX_INCLUDE_WHAT_YOU_USE
                                                       ""
)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-license>)
