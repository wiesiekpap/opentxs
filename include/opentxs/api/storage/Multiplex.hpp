// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_STORAGE_MULTIPLEX_HPP
#define OPENTXS_API_STORAGE_MULTIPLEX_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/api/storage/Driver.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Multiplex : virtual public Driver
{
public:
    virtual std::string BestRoot(bool& primaryOutOfSync) = 0;
    virtual void InitBackup() = 0;
    virtual void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) = 0;
    virtual Driver& Primary() = 0;
    virtual void SynchronizePlugins(
        const std::string& hash,
        const opentxs::storage::Root& root,
        const bool syncPrimary) = 0;

    ~Multiplex() override = default;

protected:
    Multiplex() = default;

private:
    Multiplex(const Multiplex&) = delete;
    Multiplex(Multiplex&&) = delete;
    Multiplex& operator=(const Multiplex&) = delete;
    Multiplex& operator=(Multiplex&&) = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_STORAGE_MULTIPLEX_HPP
