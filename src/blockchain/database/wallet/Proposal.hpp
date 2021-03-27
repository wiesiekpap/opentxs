// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <vector>

#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace proto
{
class BlockchainTransactionProposal;
}
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
class Proposal
{
public:
    auto CompletedProposals() const noexcept -> std::set<OTIdentifier>;
    auto Exists(const Identifier& id) const noexcept -> bool;
    auto GetMutex() const noexcept -> std::mutex&;
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal>;

    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool;
    auto CancelProposal(const Identifier& id) noexcept -> bool;
    auto FinishProposal(const Identifier& id) noexcept -> bool;
    auto ForgetProposals(const std::set<OTIdentifier>& ids) noexcept -> bool;

    Proposal() noexcept;

    ~Proposal();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
