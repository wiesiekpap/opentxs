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
    virtual auto LastSendResult() const -> SendResult = 0;
    virtual auto Reply() const -> const std::shared_ptr<Message> = 0;

    virtual auto Run(const std::size_t totalRetries = 2) -> std::string = 0;

    virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;
};
}  // namespace client
}  // namespace opentxs
#endif
