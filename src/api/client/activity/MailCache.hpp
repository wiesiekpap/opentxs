// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
class Message;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::activity
{
class MailCache
{
public:
    auto LoadMail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message>;

    auto CacheText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const std::string& text) noexcept -> void;
    auto GetText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const PasswordPrompt& reason) noexcept
        -> std::shared_future<std::string>;

    MailCache(
        const api::Core& api,
        const opentxs::network::zeromq::socket::Publish&
            messageLoaded) noexcept;

    ~MailCache();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    MailCache() = delete;
    MailCache(const MailCache&) = delete;
    MailCache(MailCache&&) = delete;
    auto operator=(const MailCache&) -> MailCache& = delete;
    auto operator=(MailCache&&) -> MailCache& = delete;
};
}  // namespace opentxs::api::client::activity
