// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_STRINGXML_HPP
#define OPENTXS_CORE_STRINGXML_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/String.hpp"

namespace irr
{
namespace io
{
class IFileReadCallBack;
}
}  // namespace irr

namespace opentxs
{
class StringXML;

using OTStringXML = Pimpl<StringXML>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT StringXML : virtual public String
{
public:
    static auto Factory() -> OTStringXML;
    static auto Factory(const String& value) -> OTStringXML;

    virtual operator irr::io::IFileReadCallBack*() = 0;

    virtual auto read(void* buffer, std::uint32_t sizeToRead)
        -> std::int32_t = 0;
    virtual auto getSize() -> std::int32_t = 0;

    virtual auto operator=(const String& rhs) -> StringXML& = 0;
    virtual auto operator=(const StringXML& rhs) -> StringXML& = 0;

    ~StringXML() override = default;

protected:
    StringXML() = default;

private:
    StringXML(StringXML&&) = delete;
    auto operator=(StringXML&&) -> StringXML& = delete;
};
}  // namespace opentxs
#endif
