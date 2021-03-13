// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_ITERATOR_BIDIRECTIONAL_HPP
#define OPENTXS_ITERATOR_BIDIRECTIONAL_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace opentxs
{
namespace iterator
{
template <typename P, typename C>
class Bidirectional
{
public:
    using value_type = C;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    OPENTXS_EXPORT Bidirectional(P* parent, std::size_t position = 0)
        : position_{position}
        , parent_{parent}
    {
    }
    OPENTXS_EXPORT Bidirectional() = default;
    OPENTXS_EXPORT Bidirectional(const Bidirectional&) = default;
    OPENTXS_EXPORT Bidirectional(Bidirectional&&) = default;
    OPENTXS_EXPORT Bidirectional& operator=(const Bidirectional&) = default;
    OPENTXS_EXPORT Bidirectional& operator=(Bidirectional&&) = default;

    OPENTXS_EXPORT reference operator*()
    {
        if (nullptr == parent_) { throw std::out_of_range{""}; }

        return parent_->at(position_);
    }

    OPENTXS_EXPORT Bidirectional& operator++()
    {
        if (std::numeric_limits<std::size_t>::max() == position_) {
            throw std::out_of_range{""};
        }

        ++position_;

        return *this;
    }
    OPENTXS_EXPORT Bidirectional operator++(int)
    {
        Bidirectional output{*this};
        ++(*this);

        return output;
    }
    OPENTXS_EXPORT Bidirectional& operator--()
    {
        if (std::numeric_limits<std::size_t>::min() == position_) {
            throw std::out_of_range{""};
        }

        --position_;

        return *this;
    }
    OPENTXS_EXPORT Bidirectional operator--(int)
    {
        const auto output{Bidirectional(*this)};
        --(*this);

        return output;
    }
    OPENTXS_EXPORT bool operator==(const Bidirectional& rhs) const
    {
        return (parent_ == rhs.parent_) && (position_ == rhs.position_);
    }
    OPENTXS_EXPORT bool operator!=(const Bidirectional& rhs) const
    {
        return !(*this == rhs);
    }

    OPENTXS_EXPORT ~Bidirectional() = default;

private:
    std::size_t position_{0};
    P* parent_{nullptr};
};
}  // namespace iterator
}  // namespace opentxs
#endif
