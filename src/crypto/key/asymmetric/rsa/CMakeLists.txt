# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(RSA_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Factory.cpp"
      "RSA.cpp"
      "RSA.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "noRSA.cpp")
endif()

if(OPENSSL_EXPORT)
  target_link_libraries(opentxs-common PRIVATE OpenSSL::Crypto)
  target_link_libraries(opentxs PUBLIC OpenSSL::Crypto)
endif()
