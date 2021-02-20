# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    NullCallback.cpp
    NymParameters.cpp
    OTCallback.cpp
    OTCaller.cpp
    OTSignatureMetadata.cpp
    OTSignedFile.cpp
    PaymentCode.cpp
    Signature.cpp
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
set(cxx-headers
    ${cxx-install-headers}
    NullCallback.hpp
    PaymentCode.hpp
    Signature.hpp
)

add_library(opentxs-core-crypto OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-core-crypto SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(
  opentxs-core-crypto PRIVATE opentxs::messages Boost::headers
)
add_dependencies(opentxs-core-crypto generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/crypto
    COMPONENT dev
  )
endif()
