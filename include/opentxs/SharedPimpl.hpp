// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SHAREDPIMPL_HPP
#define OPENTXS_SHAREDPIMPL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <memory>

#ifdef SWIG
%ignore opentxs::SharedPimpl::SharedPimpl(const std::shared_ptr<const C>&);
%ignore opentxs::SharedPimpl::SharedPimpl(SharedPimpl&&);
%ignore opentxs::SharedPimpl::operator const C&();
%rename(assign) opentxs::SharedPimpl::operator=(const SharedPimpl&);
%rename(move) opentxs::SharedPimpl::operator=(SharedPimpl&&);
#endif

namespace opentxs
{
template <class C>
class SharedPimpl
{
public:
    OPENTXS_EXPORT explicit SharedPimpl(
        const std::shared_ptr<const C>& in) noexcept
        : pimpl_(in)
    {
        if (false == bool(pimpl_)) {
            std::cout << stack_trace() << '\n';
            abort();
        }
    }
    OPENTXS_EXPORT SharedPimpl(const SharedPimpl& rhs) noexcept = default;
    OPENTXS_EXPORT SharedPimpl(SharedPimpl&& rhs) noexcept = default;
    OPENTXS_EXPORT SharedPimpl& operator=(const SharedPimpl& rhs) noexcept =
        default;
    OPENTXS_EXPORT SharedPimpl& operator=(SharedPimpl&& rhs) noexcept = default;

#ifndef SWIG
    OPENTXS_EXPORT operator const C&() const noexcept { return *pimpl_; }
#endif

    OPENTXS_EXPORT const C* operator->() const { return pimpl_.get(); }

    template <typename Type>
    OPENTXS_EXPORT SharedPimpl<Type> as() noexcept
    {
        return SharedPimpl<Type>{std::static_pointer_cast<const Type>(pimpl_)};
    }
    template <typename Type>
    OPENTXS_EXPORT SharedPimpl<Type> dynamic() noexcept(false)
    {
        auto pointer = std::dynamic_pointer_cast<const Type>(pimpl_);

        if (pointer) {
            return SharedPimpl<Type>{std::move(pointer)};
        } else {
            throw std::runtime_error("Invalid dynamic cast");
        }
    }
    OPENTXS_EXPORT const C& get() const noexcept { return *pimpl_; }

    OPENTXS_EXPORT ~SharedPimpl() = default;

#ifdef SWIG_VERSION
    OPENTXS_EXPORT SharedPimpl() = default;
#endif

private:
    std::shared_ptr<const C> pimpl_{nullptr};

#ifndef SWIG_VERSION
    SharedPimpl() = delete;
#endif
};  // class SharedPimpl
}  // namespace opentxs
#endif
