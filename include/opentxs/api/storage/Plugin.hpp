// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_STORAGE_PLUGIN_HPP
#define OPENTXS_API_STORAGE_PLUGIN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/api/storage/Driver.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Plugin : public virtual Driver
{
public:
    virtual bool EmptyBucket(const bool bucket) const override = 0;
    virtual std::string LoadRoot() const override = 0;
    virtual bool StoreRoot(const bool commit, const std::string& hash)
        const override = 0;

    ~Plugin() override = default;

protected:
    Plugin() = default;

private:
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif
