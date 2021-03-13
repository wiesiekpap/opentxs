# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-core-crypto OBJECT
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
target_link_libraries(
  opentxs-core-crypto PRIVATE opentxs::messages Boost::headers
)
target_include_directories(
  opentxs-core-crypto SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-core-crypto PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-crypto>)
add_dependencies(opentxs-core-crypto generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/crypto
    COMPONENT dev
  )
endif()
