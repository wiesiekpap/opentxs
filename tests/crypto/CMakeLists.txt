# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(unittests-opentxs-crypto-asymmetric Test_AsymmetricProvider.cpp)

if(BIP32_EXPORT)
  add_opentx_test(unittests-opentxs-crypto-bip32 Test_BIP32.cpp)
endif()

add_opentx_test(unittests-opentxs-crypto-bip39 Test_BIP39.cpp)
add_opentx_test(unittests-opentxs-crypto-bitcoin Test_BitcoinProviders.cpp)
add_opentx_test(unittests-opentxs-crypto-envelope Test_Envelope.cpp)
add_opentx_test(unittests-opentxs-crypto-hash Test_Hash.cpp)

if(OPENSSL_EXPORT)
  target_compile_definitions(
    unittests-opentxs-crypto-hash PRIVATE OT_CRYPTO_USING_OPENSSL=1
  )
else()
  target_compile_definitions(
    unittests-opentxs-crypto-hash PRIVATE OT_CRYPTO_USING_OPENSSL=0
  )
endif()
