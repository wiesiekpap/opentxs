// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Input.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "internal/blockchain/block/Block.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::blockchain::block::bitcoin::implementation
{
auto Input::Cache::add(crypto::Key&& key) noexcept -> void
{
    auto lock = rLock{lock_};
    keys_.emplace(std::move(key));
}

auto Input::Cache::associate(
    const api::client::Blockchain& blockchain,
    const internal::Output& in) noexcept -> bool
{
    auto lock = rLock{lock_};

    if (false == bool(previous_output_)) { previous_output_ = in.clone(); }

    // NOTE this should only happen during unit testing
    if (keys_.empty()) {
        auto keys = in.Keys();

        OT_ASSERT(0 < keys.size());

        std::move(keys.begin(), keys.end(), std::inserter(keys_, keys_.end()));
    }

    return bool(previous_output_);
}

auto Input::Cache::keys() const noexcept -> std::vector<crypto::Key>
{
    auto lock = rLock{lock_};
    auto output = std::vector<crypto::Key>{};
    std::transform(
        std::begin(keys_), std::end(keys_), std::back_inserter(output), [
        ](const auto& key) -> auto { return key; });

    return output;
}

auto Input::Cache::merge(
    const api::client::Blockchain& blockchain,
    const internal::Input& rhs) noexcept -> bool
{
    const auto keys = rhs.Keys();
    auto lock = rLock{lock_};
    std::copy(
        std::begin(keys), std::end(keys), std::inserter(keys_, keys_.end()));

    if (!previous_output_) {
        try {
            previous_output_ = rhs.Spends().clone();
        } catch (...) {
        }
    }

    return true;
}

auto Input::Cache::net_balance_change(
    const api::client::Blockchain& blockchain,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    auto lock = rLock{lock_};

    if (false == bool(previous_output_)) { return 0; }

    for (const auto& key : keys_) {
        if (blockchain.Owner(key) == nym) {
            return -1 * previous_output_->Value();
        }
    }

    return 0;
}

auto Input::Cache::payer() const noexcept -> OTIdentifier
{
    auto lock = rLock{lock_};

    return payer_;
}

auto Input::Cache::reset_size() noexcept -> void
{
    auto lock = rLock{lock_};
    size_ = std::nullopt;
    normalized_size_ = std::nullopt;
}

auto Input::Cache::set(const KeyData& data) noexcept -> void
{
    auto lock = rLock{lock_};

    if (payer_->empty()) {
        for (const auto& key : keys_) {
            try {
                const auto& [sender, recipient] = data.at(key);

                if (recipient->empty()) { continue; }

                payer_ = recipient;

                return;
            } catch (...) {
            }
        }
    }
}

auto Input::Cache::spends() const noexcept(false) -> const internal::Output&
{
    auto lock = rLock{lock_};

    if (previous_output_) {

        return *previous_output_;
    } else {

        throw std::runtime_error("previous output missing");
    }
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
