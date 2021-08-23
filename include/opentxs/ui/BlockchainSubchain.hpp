// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#ifndef OPENTXS_UI_BLOCKCHAINSUBCHAIN_HPP
#define OPENTXS_UI_BLOCKCHAINSUBCHAIN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/ui/ListRow.hpp"

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BlockchainSubchain : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Progress() const noexcept -> std::string = 0;
    virtual auto Type() const noexcept -> blockchain::crypto::Subchain = 0;

    ~BlockchainSubchain() override = default;

protected:
    BlockchainSubchain() noexcept = default;

private:
    BlockchainSubchain(const BlockchainSubchain&) = delete;
    BlockchainSubchain(BlockchainSubchain&&) = delete;
    auto operator=(const BlockchainSubchain&) -> BlockchainSubchain& = delete;
    auto operator=(BlockchainSubchain&&) -> BlockchainSubchain& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
