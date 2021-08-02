// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs::blockchain::block::bitcoin::implementation
{
Output::Cache::Cache(
    const api::Core& api,
    std::optional<std::size_t>&& size,
    boost::container::flat_set<KeyID>&& keys) noexcept
    : lock_()
    , size_(std::move(size))
    , payee_(api.Factory().Identifier())
    , payer_(api.Factory().Identifier())
    , keys_(std::move(keys))
{
}

Output::Cache::Cache(const Cache& rhs) noexcept
    : lock_()
    , size_()
    , payee_([&] {
        auto lock = Lock{rhs.lock_};

        return rhs.payee_;
    }())
    , payer_([&] {
        auto lock = Lock{rhs.lock_};

        return rhs.payer_;
    }())
    , keys_()
{
    auto lock = Lock{rhs.lock_};
    size_ = rhs.size_;
    keys_ = rhs.keys_;
}

auto Output::Cache::add(KeyID&& key) noexcept -> void
{
    const auto& [account, subchain, index] = key;

    if (blockchain::crypto::Subchain::Outgoing == subchain) { OT_FAIL; }

    auto lock = Lock{lock_};
    keys_.emplace(std::move(key));
}

auto Output::Cache::keys() const noexcept -> std::vector<KeyID>
{
    auto lock = Lock{lock_};
    auto output = std::vector<KeyID>{};
    std::transform(
        std::begin(keys_), std::end(keys_), std::back_inserter(output), [
        ](const auto& key) -> auto { return key; });

    return output;
}

auto Output::Cache::payee() const noexcept -> OTIdentifier
{
    auto lock = Lock{lock_};

    return payee_;
}

auto Output::Cache::payer() const noexcept -> OTIdentifier
{
    auto lock = Lock{lock_};

    return payer_;
}

auto Output::Cache::reset_size() noexcept -> void
{
    auto lock = Lock{lock_};
    size_ = std::nullopt;
}

auto Output::Cache::set(const KeyData& data) noexcept -> void
{
    auto lock = Lock{lock_};
    const auto havePayee = [&] { return !payee_->empty(); };
    const auto havePayer = [&] { return !payer_->empty(); };

    for (const auto& key : keys_) {
        if (havePayee() && havePayer()) { return; }

        try {
            const auto& [sender, recipient] = data.at(key);

            if (false == sender->empty()) {
                if (payer_->empty()) { payer_ = sender; }
            }

            if (false == recipient->empty()) {
                if (payee_->empty()) { payee_ = recipient; }
            }
        } catch (...) {
        }
    }
}

auto Output::Cache::set_payee(const Identifier& contact) noexcept -> void
{
    auto lock = Lock{lock_};

    payee_ = contact;
}

auto Output::Cache::set_payer(const Identifier& contact) noexcept -> void
{
    auto lock = Lock{lock_};

    payer_ = contact;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
