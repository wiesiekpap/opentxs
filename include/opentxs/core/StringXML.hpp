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
class StringXML : virtual public String
{
public:
    OPENTXS_EXPORT static OTStringXML Factory();
    OPENTXS_EXPORT static OTStringXML Factory(const String& value);

    OPENTXS_EXPORT virtual operator irr::io::IFileReadCallBack*() = 0;

    OPENTXS_EXPORT virtual std::int32_t read(
        void* buffer,
        std::uint32_t sizeToRead) = 0;
    OPENTXS_EXPORT virtual std::int32_t getSize() = 0;

    OPENTXS_EXPORT virtual StringXML& operator=(const String& rhs) = 0;
    OPENTXS_EXPORT virtual StringXML& operator=(const StringXML& rhs) = 0;

    OPENTXS_EXPORT ~StringXML() override = default;

protected:
    StringXML() = default;

private:
    StringXML(StringXML&&) = delete;
    StringXML& operator=(StringXML&&) = delete;
};
}  // namespace opentxs
#endif
