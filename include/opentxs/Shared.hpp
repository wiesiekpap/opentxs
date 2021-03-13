// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SHARED_HPP
#define OPENTXS_SHARED_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cassert>  // IWYU pragma: keep
#include <memory>
#include <shared_mutex>
#include <stdexcept>

#include "opentxs/Types.hpp"

#ifdef SWIG
%ignore opentxs::Shared::Shared(Shared&&);
%rename(assign) opentxs::Shared::operator=(const Shared&);
%rename(move) opentxs::Shared::operator=(Shared&&);
%rename(valid) opentxs::Shared::operator bool();
#endif

namespace opentxs
{
template <class C>
class Shared
{
public:
    OPENTXS_EXPORT operator bool() const { return nullptr != p_; }
#ifndef SWIG
    OPENTXS_EXPORT operator const C&() const { return get(); }
#endif

    OPENTXS_EXPORT const C& get() const
    {
        if (nullptr == p_) { throw std::runtime_error("Invalid pointer"); }

        return *p_;
    }

    OPENTXS_EXPORT bool Release() noexcept
    {
        if (nullptr == p_) { return false; }

        p_ = nullptr;
        lock_.reset(nullptr);

        return true;
    }

    OPENTXS_EXPORT Shared(const C* in, std::shared_mutex& lock) noexcept
        : p_(in)
        , lock_(new sLock(lock))
    {
        assert(lock_);
    }
    OPENTXS_EXPORT Shared() noexcept
        : p_(nullptr)
        , lock_(nullptr)
    {
    }

    OPENTXS_EXPORT Shared(const Shared& rhs) noexcept
        : p_(rhs.p_)
        , lock_(
              (nullptr != rhs.lock_->mutex()) ? new sLock(*rhs.lock_->mutex())
                                              : nullptr)
    {
    }
    OPENTXS_EXPORT Shared(Shared&& rhs) noexcept
        : p_(rhs.p_)
        , lock_(rhs.lock_.release())
    {
        rhs.p_ = nullptr;
    }
    OPENTXS_EXPORT Shared& operator=(const Shared& rhs) noexcept
    {
        p_ = rhs.p_;

        if (nullptr != rhs.lock_->mutex()) {
            lock_.reset(new sLock(*rhs.lock_->mutex()));
        } else {
            lock_.reset(nullptr);
        }

        return *this;
    }
    OPENTXS_EXPORT Shared& operator=(Shared&& rhs) noexcept
    {
        p_ = rhs.p_;
        rhs.p_ = nullptr;
        lock_.reset(rhs.lock_.release());

        return *this;
    }

    OPENTXS_EXPORT ~Shared() { Release(); }

private:
    const C* p_{nullptr};
    std::unique_ptr<sLock> lock_{nullptr};

};  // class Shared
}  // namespace opentxs
#endif
