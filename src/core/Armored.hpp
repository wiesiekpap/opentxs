// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "String.hpp"
#include "opentxs/core/Armored.hpp"

namespace opentxs
{
namespace OTDB
{
class OTPacker;
}  // namespace OTDB

namespace crypto
{
class Envelope;
}  // namespace crypto

class Data;
class Factory;
class String;
}  // namespace opentxs

namespace opentxs::implementation
{
class Armored : virtual public opentxs::Armored, public String
{
public:
    auto GetData(Data& theData, bool bLineBreaks = true) const -> bool override;
    auto GetString(opentxs::String& theData, bool bLineBreaks = true) const
        -> bool override;
    auto WriteArmoredString(
        opentxs::String& strOutput,
        const std::string str_type,
        bool bEscaped = false) const -> bool override;
    auto LoadFrom_ifstream(std::ifstream& fin) -> bool override;
    auto LoadFromExactPath(const std::string& filename) -> bool override;
    auto LoadFromString(
        opentxs::String& theStr,
        bool bEscaped = false,
        const std::string str_override = "-----BEGIN") -> bool override;
    auto SaveTo_ofstream(std::ofstream& fout) -> bool override;
    auto SaveToExactPath(const std::string& filename) -> bool override;
    auto SetData(const Data& theData, bool bLineBreaks = true) -> bool override;
    auto SetString(const opentxs::String& theData, bool bLineBreaks = true)
        -> bool override;

    ~Armored() override = default;

protected:
    Armored();

private:
    friend OTArmored;
    friend opentxs::Armored;
    friend opentxs::Factory;

    static std::unique_ptr<OTDB::OTPacker> s_pPacker;

    auto clone() const -> Armored* override;
    auto compress_string(const std::string& str, std::int32_t compressionlevel)
        const -> std::string;
    auto decompress_string(const std::string& str) const -> std::string;

    explicit Armored(const Data& theValue);
    explicit Armored(const opentxs::String& strValue);
    explicit Armored(const crypto::Envelope& theEnvelope);
    Armored(const Armored& strValue);

    auto operator=(const char* szValue) -> Armored&;
    auto operator=(const Data& theValue) -> Armored&;
    auto operator=(const opentxs::String& strValue) -> Armored&;
    auto operator=(const Armored& strValue) -> Armored&;
};
}  // namespace opentxs::implementation
