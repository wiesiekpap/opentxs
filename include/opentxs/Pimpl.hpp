// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PIMPL_HPP
#define OPENTXS_PIMPL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cassert>
#include <memory>

#ifdef SWIG
%ignore opentxs::Pimpl::Pimpl(Pimpl&&);
%ignore opentxs::Pimpl::operator->();
%ignore opentxs::Pimpl::get() const;
%rename(assign) opentxs::Pimpl::operator=(const Pimpl&);
%rename(move) opentxs::Pimpl::operator=(Pimpl&&);
#endif

namespace opentxs
{
template <class C>
class Pimpl
{
public:
    using interface_type = C;

    OPENTXS_EXPORT explicit Pimpl(C* in) noexcept
        : pimpl_(in)
    {
        assert(pimpl_);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    OPENTXS_EXPORT Pimpl(const Pimpl& rhs) noexcept
        : pimpl_(
#ifndef _WIN32
              rhs.pimpl_->clone()
#else
              dynamic_cast<C*>(rhs.pimpl_->clone())
#endif
          )
    {
        assert(pimpl_);
    }
#pragma GCC diagnostic pop

    OPENTXS_EXPORT Pimpl(const C& rhs) noexcept
        : pimpl_(
#ifndef _WIN32
              rhs.clone()
#else
              dynamic_cast<C*>(rhs.clone())
#endif
          )
    {
        assert(pimpl_);
    }

    OPENTXS_EXPORT Pimpl(Pimpl&& rhs) noexcept
        : pimpl_(std::move(rhs.pimpl_))
    {
        assert(pimpl_);
    }

    OPENTXS_EXPORT Pimpl(std::unique_ptr<C>&& rhs) noexcept
        : pimpl_(std::move(rhs))
    {
        assert(pimpl_);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    OPENTXS_EXPORT Pimpl& operator=(const Pimpl& rhs) noexcept
    {
        pimpl_.reset(
#ifndef _WIN32
            rhs.pimpl_->clone()
#else
            dynamic_cast<C*>(rhs.pimpl_->clone())
#endif
        );
        assert(pimpl_);

        return *this;
    }
#pragma GCC diagnostic pop

    OPENTXS_EXPORT Pimpl& operator=(Pimpl&& rhs) noexcept
    {
        pimpl_ = std::move(rhs.pimpl_);
        assert(pimpl_);

        return *this;
    }

    OPENTXS_EXPORT Pimpl& operator=(const C& rhs) noexcept
    {
        pimpl_.reset(
#ifndef _WIN32
            rhs.clone()
#else
            dynamic_cast<C*>(rhs.clone())
#endif
        );
        assert(pimpl_);

        return *this;
    }

    OPENTXS_EXPORT operator C&() noexcept { return *pimpl_; }
    OPENTXS_EXPORT operator const C&() const noexcept { return *pimpl_; }

    OPENTXS_EXPORT C* operator->() { return pimpl_.get(); }
    OPENTXS_EXPORT const C* operator->() const { return pimpl_.get(); }

    OPENTXS_EXPORT C& get() noexcept { return *pimpl_; }
    OPENTXS_EXPORT const C& get() const noexcept { return *pimpl_; }

    OPENTXS_EXPORT ~Pimpl() = default;

#if defined SWIG_VERSION || defined _WIN32
    OPENTXS_EXPORT Pimpl() = default;
#endif

private:
    std::unique_ptr<C> pimpl_{nullptr};

#if !defined SWIG_VERSION && !defined _WIN32
    Pimpl() = delete;
#endif
};  // class Pimpl
}  // namespace opentxs

#endif
