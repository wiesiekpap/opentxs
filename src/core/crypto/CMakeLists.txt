# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "NullCallback.cpp"
    "NullCallback.hpp"
    "NymParameters.cpp"
    "OTCallback.cpp"
    "OTCaller.cpp"
    "OTSignatureMetadata.cpp"
    "OTSignedFile.cpp"
    "PaymentCode.cpp"
    "PaymentCode.hpp"
    "Signature.cpp"
    "Signature.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/NymParameters.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/OTCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/OTCaller.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/OTSignatureMetadata.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/OTSignedFile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/PaymentCode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/crypto/Signature.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/crypto
    COMPONENT dev
  )
endif()
