// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/crypto/library/HashingProvider.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs::crypto
{
auto HashingProvider::StringToHashType(const String& inputString)
    -> proto::HashType
{
    if (inputString.Compare("NULL")) {
        return proto::HASHTYPE_NONE;
    } else if (inputString.Compare("SHA256")) {
        return proto::HASHTYPE_SHA256;
    } else if (inputString.Compare("SHA512")) {
        return proto::HASHTYPE_SHA512;
    } else if (inputString.Compare("BLAKE2B160")) {
        return proto::HASHTYPE_BLAKE2B160;
    } else if (inputString.Compare("BLAKE2B256")) {
        return proto::HASHTYPE_BLAKE2B256;
    } else if (inputString.Compare("BLAKE2B512")) {
        return proto::HASHTYPE_BLAKE2B512;
    }

    return proto::HASHTYPE_ERROR;
}
auto HashingProvider::HashTypeToString(const proto::HashType hashType)
    -> OTString

{
    auto hashTypeString = String::Factory();

    switch (hashType) {
        case proto::HASHTYPE_NONE: {
            hashTypeString = String::Factory("NULL");
        } break;
        case proto::HASHTYPE_SHA256: {
            hashTypeString = String::Factory("SHA256");
        } break;
        case proto::HASHTYPE_SHA512: {
            hashTypeString = String::Factory("SHA512");
        } break;
        case proto::HASHTYPE_BLAKE2B160: {
            hashTypeString = String::Factory("BLAKE2B160");
        } break;
        case proto::HASHTYPE_BLAKE2B256: {
            hashTypeString = String::Factory("BLAKE2B256");
        } break;
        case proto::HASHTYPE_BLAKE2B512: {
            hashTypeString = String::Factory("BLAKE2B512");
        } break;
        default: {
            hashTypeString = String::Factory("ERROR");
        }
    }

    return hashTypeString;
}

auto HashingProvider::HashSize(const proto::HashType hashType) -> std::size_t
{
    switch (hashType) {
        case proto::HASHTYPE_SHA256: {
            return 32;
        }
        case proto::HASHTYPE_SHA512: {
            return 64;
        }
        case proto::HASHTYPE_BLAKE2B160: {
            return 20;
        }
        case proto::HASHTYPE_BLAKE2B256: {
            return 32;
        }
        case proto::HASHTYPE_BLAKE2B512: {
            return 64;
        }
        case proto::HASHTYPE_RIPEMD160: {
            return 20;
        }
        case proto::HASHTYPE_SHA1: {
            return 20;
        }
        case proto::HASHTYPE_SHA256D: {
            return 32;
        }
        case proto::HASHTYPE_SHA256DC: {
            return 4;
        }
        case proto::HASHTYPE_BITCOIN: {
            return 20;
        }
        case proto::HASHTYPE_SIPHASH24: {
            return 8;
        }
        default: {
        }
    }

    return 0;
}
}  // namespace opentxs::crypto
