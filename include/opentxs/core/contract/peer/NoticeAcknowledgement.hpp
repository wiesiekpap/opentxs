// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/PeerReply.hpp"

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
class Acknowledgement;
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTReplyAcknowledgement =
    SharedPimpl<contract::peer::reply::Acknowledgement>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
class OPENTXS_EXPORT Acknowledgement : virtual public peer::Reply
{
public:
    ~Acknowledgement() override = default;

protected:
    Acknowledgement() noexcept = default;

private:
    friend OTReplyAcknowledgement;

#ifndef _WIN32
    auto clone() const noexcept -> Acknowledgement* override = 0;
#endif

    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;
};
}  // namespace reply
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
