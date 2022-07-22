// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string_view>

#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Cfilter;
}  // namespace database

namespace node
{
class FilterOracle;
class Manager;
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
class BlockIndexer
{
public:
    auto Reindex() noexcept -> void;
    auto Start() noexcept -> void;

    BlockIndexer(
        const api::Session& api,
        const node::Manager& node,
        const node::FilterOracle& parent,
        database::Cfilter& db,
        NotifyCallback&& notify,
        blockchain::Type chain,
        cfilter::Type type,
        std::string_view parentEndpoint) noexcept;
    BlockIndexer(const BlockIndexer&) = delete;
    BlockIndexer(BlockIndexer&&) = delete;
    auto operator=(const BlockIndexer&) -> BlockIndexer& = delete;
    auto operator=(BlockIndexer&&) -> BlockIndexer& = delete;

    ~BlockIndexer();

private:
    class Imp;

    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::filteroracle
