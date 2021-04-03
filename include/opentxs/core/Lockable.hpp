// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LOCKABLE_HPP
#define OPENTXS_CORE_LOCKABLE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <mutex>
#include <shared_mutex>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
template <typename L, typename M>
bool CheckLock(const L& lock, const M& mutex) noexcept
{
    if (lock.mutex() != &mutex) {
        LogOutput(": Lock is on incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(": Lock is unlocked.").Flush();

        return false;
    }

    return true;
}

class Lockable
{
public:
    Lockable() noexcept
        : lock_()
        , shared_lock_()
    {
    }

    virtual ~Lockable() = default;

protected:
    mutable std::mutex lock_;
    mutable std::shared_mutex shared_lock_;

    bool verify_lock(const Lock& lock) const noexcept
    {
        return CheckLock(lock, lock_);
    }

    bool verify_lock(const sLock& lock) const noexcept
    {
        return CheckLock(lock, shared_lock_);
    }

    bool verify_lock(const eLock& lock) const noexcept
    {
        return CheckLock(lock, shared_lock_);
    }

private:
    Lockable(const Lockable&) = delete;
    Lockable(Lockable&&) = delete;
    Lockable& operator=(const Lockable&) = delete;
    Lockable& operator=(Lockable&&) = delete;
};
}  // namespace opentxs

#endif
