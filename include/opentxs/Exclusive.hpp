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

#ifdef SWIG
%ignore opentxs::Exclusive::Exclusive(Exclusive&&);
%rename(move) opentxs::Exclusive::operator=(Exclusive&&);
%rename(valid) opentxs::Exclusive::operator bool();
#endif

namespace opentxs
{
template <typename C>
class Exclusive
{
public:
    using Callback = std::function<void(const C&)>;
    using Container = std::unique_ptr<C>;
    using Save = std::function<void(Container&, eLock&, bool)>;

    OPENTXS_EXPORT operator bool() const;
#ifndef SWIG
    OPENTXS_EXPORT operator const C&() const;
    OPENTXS_EXPORT const C& get() const;

    OPENTXS_EXPORT operator C&();
#endif

    OPENTXS_EXPORT bool Abort();
    OPENTXS_EXPORT C& get();
    OPENTXS_EXPORT bool Release();

    OPENTXS_EXPORT Exclusive(
        Container* in,
        std::shared_mutex& lock,
        Save save,
        const Callback callback = nullptr) noexcept;
    OPENTXS_EXPORT Exclusive() noexcept;
    Exclusive(const Exclusive&) = delete;
    OPENTXS_EXPORT Exclusive(Exclusive&&) noexcept;
    Exclusive& operator=(const Exclusive&) noexcept = delete;
    OPENTXS_EXPORT Exclusive& operator=(Exclusive&&) noexcept;

    OPENTXS_EXPORT ~Exclusive();

private:
    Container* p_{nullptr};
    std::unique_ptr<eLock> lock_{nullptr};
    Save save_{[](Container&, eLock&, bool) -> void {}};
    std::atomic<bool> success_{true};
    Callback callback_{nullptr};
};  // class Exclusive
}  // namespace opentxs
#endif
