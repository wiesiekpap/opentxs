// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "opentxs/util/Container.hpp"  // IWYU pragma: associated

#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>

namespace opentxs::alloc
{
class ForwardToBoost final : public Resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        return boost_->allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        return boost_->deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {
        return &other == this;
    }

    ForwardToBoost(boost::container::pmr::memory_resource* boost) noexcept
        : boost_(boost)
    {
    }
    ForwardToBoost(const ForwardToBoost&) = delete;
    ForwardToBoost(ForwardToBoost&&) = delete;
    auto operator=(const ForwardToBoost&) -> ForwardToBoost& = delete;
    auto operator=(ForwardToBoost&&) -> ForwardToBoost& = delete;

    ~ForwardToBoost() final = default;

private:
    boost::container::pmr::memory_resource* boost_;
};

auto System() noexcept -> Resource*
{
    // TODO replace with std::pmr::new_delete_resource once Android and Apple
    // catch up
    static auto resource =
        ForwardToBoost{boost::container::pmr::new_delete_resource()};

    return &resource;
}

auto Null() noexcept -> Resource*
{
    // TODO replace with std::pmr::null_memory_resource once Android and Apple
    // catch up
    static auto resource =
        ForwardToBoost{boost::container::pmr::null_memory_resource()};

    return &resource;
}
}  // namespace opentxs::alloc
