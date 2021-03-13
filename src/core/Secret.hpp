// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sodium.h>
}

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <optional>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs::implementation
{
template <typename T>
struct SecureAllocator {
    using value_type = T;

    SecureAllocator()
    {
        if (0 > ::sodium_init()) { throw std::bad_alloc(); }
    }

    template <class U>
    SecureAllocator(const SecureAllocator<U>&)
    {
        if (0 > ::sodium_init()) { throw std::bad_alloc(); }
    }

    auto allocate(const std::size_t items) -> value_type*
    {
        constexpr auto limit =
            std::numeric_limits<std::size_t>::max() / sizeof(value_type);

        if (items > limit) { throw std::bad_alloc(); }

        const auto bytes = items * sizeof(value_type);
        auto output = std::malloc(bytes);

        if (nullptr == output) { throw std::bad_alloc(); }

        static auto warn{false};

        if (0 > ::sodium_mlock(output, bytes)) {
            if (false == warn) {
                LogVerbose("Unable to lock memory. Passwords and/or secret "
                           "keys may be swapped to disk")
                    .Flush();
            }

            warn = true;
        } else {
            warn = false;
        }

        return static_cast<value_type*>(output);
    }
    auto deallocate(value_type* in, const std::size_t bytes) -> void
    {
        ::sodium_munlock(in, bytes);
        std::free(in);
    }
};

template <typename T, typename U>
bool operator==(const SecureAllocator<T>&, const SecureAllocator<U>&)
{
    return true;
}

template <typename T, typename U>
bool operator!=(const SecureAllocator<T>& lhs, const SecureAllocator<U>& rhs)
{
    return !(lhs == rhs);
}

class Secret final : virtual public opentxs::Secret
{
public:
    auto operator==(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator==(rhs.Bytes());
    }
    auto operator==(const ReadView rhs) const noexcept -> bool final;
    auto operator!=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator!=(rhs.Bytes());
    }
    auto operator!=(const ReadView rhs) const noexcept -> bool final;
    auto operator<(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator<(rhs.Bytes());
    }
    auto operator<(const ReadView rhs) const noexcept -> bool final;
    auto operator>(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator>(rhs.Bytes());
    }
    auto operator>(const ReadView rhs) const noexcept -> bool final;
    auto operator<=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator<=(rhs.Bytes());
    }
    auto operator<=(const ReadView& rhs) const noexcept -> bool final;
    auto operator>=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator>=(rhs.Bytes());
    }
    auto operator>=(const ReadView& rhs) const noexcept -> bool final;
    auto Bytes() const noexcept -> ReadView final;
    auto data() const noexcept -> const std::byte* final
    {
        return data_.data();
    }
    auto empty() const noexcept -> bool final { return data_.empty(); }
    auto size() const noexcept -> std::size_t final;

    auto operator+=(const opentxs::Secret& rhs) noexcept -> Secret& final;
    auto operator+=(const ReadView rhs) noexcept -> Secret& final;

    auto Assign(const opentxs::Secret& rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Assign(const ReadView rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Assign(const void* data, const std::size_t& size) noexcept
        -> void final;
    auto AssignText(const ReadView source) noexcept -> void final;
    auto clear() noexcept -> void final { data_.clear(); }
    auto Concatenate(const opentxs::Secret& rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Concatenate(const ReadView rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept
        -> void final;
    auto data() noexcept -> std::byte* final { return data_.data(); }
    auto Randomize(const std::size_t bytes) noexcept -> std::size_t final;
    auto Resize(const std::size_t size) noexcept -> std::size_t final;
    auto WriteInto(const std::optional<Mode> = {}) noexcept
        -> AllocateOutput final;

    Secret(const std::size_t bytes) noexcept;
    Secret(const ReadView bytes, const Mode mode) noexcept;
    ~Secret() final = default;

private:
    friend opentxs::Secret;

    static const SecureAllocator<std::byte> allocator_;

    Mode mode_;
    std::vector<std::byte, SecureAllocator<std::byte>> data_;

    auto clone() const noexcept -> Secret* final { return new Secret(*this); }
    auto mem() const noexcept -> bool { return Mode::Mem == mode_; }
    auto spaceship(const ReadView rhs) const noexcept -> int;

    Secret() = delete;
    Secret(const Secret& rhs) noexcept;
    Secret(Secret&& rhs) = delete;
    auto operator=(const Secret& rhs) -> Secret& = delete;
    auto operator=(Secret&& rhs) -> Secret& = delete;
};
}  // namespace opentxs::implementation
