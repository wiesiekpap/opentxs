# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-test-helpers
  PRIVATE
    "VectorsV1.hpp"
    "VectorsV3.hpp"
    "Vectors.cpp"
)

if(SECP256K1_EXPORT AND BIP32_EXPORT)
  target_sources(opentxs-test-helpers PRIVATE "Helpers.hpp" "Helpers.cpp")

  add_opentx_test(unittests-opentxs-paymentcode-api API.cpp)
  add_opentx_test(unittests-opentxs-paymentcode-v1 Version_1.cpp)
  add_opentx_test(unittests-opentxs-paymentcode-v3 Version_3.cpp)
  add_opentx_test(unittests-opentxs-paymentcode-v1-v3-compat V1_V3.cpp)
endif()
