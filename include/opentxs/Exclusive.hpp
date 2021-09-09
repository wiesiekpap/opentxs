// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::Account

#ifndef OPENTXS_EXCLUSIVE_HPP
#define OPENTXS_EXCLUSIVE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <atomic>
#include <functional>
#include <memory>
#include <shared_mutex>

#include "opentxs/Types.hpp"

namespace opentxs
{
template <typename C>
class OPENTXS_EXPORT Exclusive
{
public:
    using Callback = std::function<void(const C&)>;
    using Container = std::unique_ptr<C>;
    using Save = std::function<void(Container&, eLock&, bool)>;

    operator bool() const;
    operator const C&() const;
    auto get() const -> const C&;

    operator C&();
    auto Abort() -> bool;
    auto get() -> C&;
    auto Release() -> bool;

    Exclusive(
        Container* in,
        std::shared_mutex& lock,
        Save save,
        const Callback callback = nullptr) noexcept;
    Exclusive() noexcept;
    Exclusive(Exclusive&&) noexcept;
    auto operator=(Exclusive&&) noexcept -> Exclusive&;

    ~Exclusive();

private:
    Container* p_{nullptr};
    std::unique_ptr<eLock> lock_{nullptr};
    Save save_{[](Container&, eLock&, bool) -> void {}};
    std::atomic<bool> success_{true};
    Callback callback_{nullptr};

    Exclusive(const Exclusive&) = delete;
    auto operator=(const Exclusive&) noexcept -> Exclusive& = delete;
};  // class Exclusive
}  // namespace opentxs
#endif
