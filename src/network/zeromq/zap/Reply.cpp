// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "network/zeromq/zap/Reply.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "network/zeromq/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Reply>;

namespace opentxs::network::zeromq::zap
{
auto Reply::Factory(
    const Request& request,
    const zap::Status& code,
    const std::string& status,
    const std::string& userID,
    const Data& metadata,
    const std::string& version) -> OTZMQZAPReply
{
    return OTZMQZAPReply(new implementation::Reply(
        request, code, status, userID, metadata, version));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
const Reply::CodeMap Reply::code_map_{
    {Status::Success, "200"},
    {Status::TemporaryError, "300"},
    {Status::AuthFailure, "400"},
    {Status::SystemError, "500"},
};
const Reply::CodeReverseMap Reply::code_reverse_map_{
    invert_code_map(code_map_)};

Reply::Reply(
    const Request& request,
    const zap::Status& code,
    const std::string& status,
    const std::string& userID,
    const Data& metadata,
    const std::string& version)
    : zeromq::implementation::Message()
{
    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) {
            messages_.emplace_back(frame);
        }
    }

    AddFrame();
    zeromq::Message::AddFrame(version);
    zeromq::Message::AddFrame(request.RequestID());
    zeromq::Message::AddFrame(code_to_string(code));
    zeromq::Message::AddFrame(status);
    zeromq::Message::AddFrame(userID);
    zeromq::Message::AddFrame(metadata);
}

Reply::Reply(const Reply& rhs)
    : zeromq::Message()
    , zeromq::zap::Reply()
    , zeromq::implementation::Message()
{
    messages_ = rhs.messages_;
}

auto Reply::code_to_string(const zap::Status& code) -> std::string
{
    try {
        return code_map_.at(code);
    } catch (...) {

        return "";
    }
}

auto Reply::invert_code_map(const CodeMap& input) -> Reply::CodeReverseMap
{
    CodeReverseMap output{};

    for (const auto& [type, string] : input) { output.emplace(string, type); }

    return output;
}

auto Reply::Debug() const -> std::string
{
    std::stringstream output{};
    const auto body = Body();

    if (VERSION_POSITION < body.size()) {
        output << "Version: " << std::string(body.at(VERSION_POSITION)) << "\n";
    }

    if (REQUEST_ID_POSITION < body.size()) {
        const auto& requestID = body.at(REQUEST_ID_POSITION);

        if (0 < requestID.size()) {
            output << "Request ID: 0x" << Data::Factory(requestID)->asHex()
                   << "\n";
        }
    }

    if (STATUS_CODE_POSITION < body.size()) {
        output << "Status Code: " << std::string(body.at(STATUS_CODE_POSITION))
               << "\n";
    }

    if (STATUS_TEXT_POSITION < body.size()) {
        output << "Status Text: " << std::string(body.at(STATUS_TEXT_POSITION))
               << "\n";
    }

    if (USER_ID_POSITION < body.size()) {
        output << "User ID: " << std::string(body.at(USER_ID_POSITION)) << "\n";
    }

    if (METADATA_POSITION < body.size()) {
        const auto& metadata = body.at(METADATA_POSITION);

        if (0 < metadata.size()) {
            output << "Metadata: 0x" << Data::Factory(metadata)->asHex()
                   << "\n";
        }
    }

    return output.str();
}

auto Reply::Metadata() const -> OTData
{
    const auto& frame = Body_at(METADATA_POSITION);

    return Data::Factory(frame.data(), frame.size());
}

auto Reply::RequestID() const -> OTData
{
    const auto& frame = Body_at(REQUEST_ID_POSITION);

    return Data::Factory(frame.data(), frame.size());
}

auto Reply::string_to_code(const std::string& string) -> zap::Status
{
    try {
        return code_reverse_map_.at(string);
    } catch (...) {

        return Status::Unknown;
    }
}
}  // namespace opentxs::network::zeromq::zap::implementation
