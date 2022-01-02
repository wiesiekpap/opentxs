# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/crypto/OTSignatureMetadata.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/crypto/OTSignedFile.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/crypto/Signature.hpp"
    "OTSignatureMetadata.cpp"
    "OTSignedFile.cpp"
    "Signature.cpp"
    "Signature.hpp"
)
