// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_SERVERACTION_HPP
#define OPENTXS_CLIENT_SERVERACTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace client
{
class OPENTXS_EXPORT ServerAction
{
public:
    virtual SendResult LastSendResult() const = 0;
    virtual const std::shared_ptr<Message> Reply() const = 0;

    virtual std::string Run(const std::size_t totalRetries = 2) = 0;

    virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace client
}  // namespace opentxs
#endif
