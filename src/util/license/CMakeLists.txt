# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/base58.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/base64.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/bech32.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/chai.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/irrxml.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/libguarded.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/lucre.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/opentxs.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/packetcrypt.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/secp256k1.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/simpleini.cpp"
    "License.cpp"
    "License.hpp"
    "argon.cpp"
    "lgpl21.cpp"
    "mpl2.cpp"
)
