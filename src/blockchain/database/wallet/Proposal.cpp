// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Proposal.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"

// #define OT_METHOD "opentxs::blockchain::database::Proposal::"

namespace opentxs::blockchain::database::wallet
{
struct Proposal::Imp {
    auto CompletedProposals() const noexcept -> std::set<OTIdentifier>
    {
        auto lock = Lock{lock_};

        return finished_proposals_;
    }
    auto Exists(const Identifier& id) const noexcept -> bool
    {
        auto lock = Lock{lock_};

        return proposals_.count(id);
    }
    auto GetMutex() const noexcept -> std::mutex& { return lock_; }
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        auto lock = Lock{lock_};

        return load_proposal(lock, id);
    }
    auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal>
    {
        auto output = std::vector<proto::BlockchainTransactionProposal>{};
        auto lock = Lock{lock_};
        std::transform(
            std::begin(proposals_),
            std::end(proposals_),
            std::back_inserter(output),
            [](const auto& in) -> auto { return in.second; });

        return output;
    }

    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool
    {
        auto lock = Lock{lock_};
        proposals_[id] = tx;

        return true;
    }
    auto CancelProposal(const Identifier& id) noexcept -> bool
    {
        auto lock = Lock{lock_};
        proposals_.erase(id);

        return true;
    }
    auto FinishProposal(const Identifier& id) noexcept -> bool
    {
        auto lock = Lock{lock_};
        proposals_.erase(id);
        finished_proposals_.emplace(id);

        return true;
    }
    auto ForgetProposals(const std::set<OTIdentifier>& ids) noexcept -> bool
    {
        auto lock = Lock{lock_};

        for (const auto& id : ids) { finished_proposals_.erase(id); }

        return true;
    }

    Imp() noexcept
        : lock_()
        , proposals_()
        , finished_proposals_()
    {
    }

private:
    using ProposalMap =
        std::map<OTIdentifier, proto::BlockchainTransactionProposal>;
    using FinishedProposals = std::set<OTIdentifier>;

    mutable std::mutex lock_;
    mutable ProposalMap proposals_;
    mutable FinishedProposals finished_proposals_;  // NOTE don't move to lmdb

    auto load_proposal(const Lock& lock, const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        auto it = proposals_.find(id);

        if (proposals_.end() == it) { return std::nullopt; }

        return it->second;
    }
};

Proposal::Proposal() noexcept
    : imp_(std::make_unique<Imp>())
{
    OT_ASSERT(imp_);
}

auto Proposal::AddProposal(
    const Identifier& id,
    const proto::BlockchainTransactionProposal& tx) noexcept -> bool
{
    return imp_->AddProposal(id, tx);
}

auto Proposal::CancelProposal(const Identifier& id) noexcept -> bool
{
    return imp_->CancelProposal(id);
}

auto Proposal::CompletedProposals() const noexcept -> std::set<OTIdentifier>
{
    return imp_->CompletedProposals();
}

auto Proposal::Exists(const Identifier& id) const noexcept -> bool
{
    return imp_->Exists(id);
}

auto Proposal::FinishProposal(const Identifier& id) noexcept -> bool
{
    return imp_->FinishProposal(id);
}

auto Proposal::ForgetProposals(const std::set<OTIdentifier>& ids) noexcept
    -> bool
{
    return imp_->ForgetProposals(ids);
}

auto Proposal::GetMutex() const noexcept -> std::mutex&
{
    return imp_->GetMutex();
}

auto Proposal::LoadProposal(const Identifier& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposal(id);
}

auto Proposal::LoadProposals() const noexcept
    -> std::vector<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposals();
}

Proposal::~Proposal() = default;
}  // namespace opentxs::blockchain::database::wallet
