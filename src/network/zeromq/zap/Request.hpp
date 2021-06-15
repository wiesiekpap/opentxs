// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>

#include "network/zeromq/Message.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/network/zeromq/zap/ZAP.hpp"

#define VERSION_POSITION 0
#define REQUEST_ID_POSITION 1
#define DOMAIN_POSITION 2
#define ADDRESS_POSITION 3
#define IDENTITY_POSITION 4
#define MECHANISM_POSITION 5
#define CREDENTIALS_START_POSITION 6

namespace opentxs::network::zeromq::zap::implementation
{
class Request final : virtual zap::Request, zeromq::implementation::Message
{
public:
    auto Address() const -> std::string final
    {
        return Body_at(ADDRESS_POSITION);
    }
    auto Credentials() const -> const FrameSection final;
    auto Debug() const -> std::string final;
    auto Domain() const -> std::string final
    {
        return Body_at(DOMAIN_POSITION);
    }
    auto Identity() const -> OTData final
    {
        return Data::Factory(Body_at(IDENTITY_POSITION));
    }
    auto Mechanism() const -> zap::Mechanism final
    {
        return string_to_mechanism(Body_at(MECHANISM_POSITION));
    }
    auto RequestID() const -> OTData final
    {
        return Data::Factory(Body_at(REQUEST_ID_POSITION));
    }
    auto Validate() const -> std::pair<bool, std::string> final;
    auto Version() const -> std::string final
    {
        return Body_at(VERSION_POSITION);
    }

    auto AddCredential(const Data& credential) -> Frame& final
    {
        return zeromq::Message::AddFrame(credential);
    }

    ~Request() final = default;

private:
    friend network::zeromq::zap::Request;

    using MechanismMap = std::map<zap::Mechanism, std::string>;
    using MechanismReverseMap = std::map<std::string, zap::Mechanism>;

    static const std::set<std::string> accept_versions_;
    static const MechanismMap mechanism_map_;
    static const MechanismReverseMap mechanism_reverse_map_;

    static auto invert_mechanism_map(const MechanismMap& input)
        -> MechanismReverseMap;
    static auto mechanism_to_string(const zap::Mechanism in) -> std::string;
    static auto string_to_mechanism(const std::string& in) -> zap::Mechanism;

    auto clone() const -> Request* final { return new Request(*this); }

    Request(
        const std::string& address,
        const std::string& domain,
        const zap::Mechanism mechanism,
        const Data& requestID,
        const Data& identity,
        const std::string& version);
    Request();
    Request(const Request&);
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
