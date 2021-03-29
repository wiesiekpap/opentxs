// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <vector>

namespace opentxs
{
template <typename T>
auto contains(const std::vector<T>& vector, const T& value) noexcept -> bool
{
    for (const auto& item : vector) {
        if (item == value) { return true; }
    }

    return false;
}

template <typename T>
auto dedup(std::vector<T>& vector) noexcept -> void
{
    std::sort(vector.begin(), vector.end());
    vector.erase(std::unique(vector.begin(), vector.end()), vector.end());
}

template <typename Store, typename Interface>
auto insert_sorted(std::vector<Store>& vector, const Interface& key) noexcept
    -> void
{
    static const auto less = std::less<Interface>{};

    for (auto i{vector.begin()}; i != vector.end(); std::advance(i, 1)) {
        const auto& val = *i;

        if (less(val, key)) {

            continue;
        } else if (less(key, val)) {
            vector.insert(i, key);

            return;
        } else {

            return;
        }
    }

    vector.emplace_back(key);
}

template <typename Store, typename Interface>
auto remove(std::vector<Store>& vector, const Interface& key) noexcept
    -> std::size_t
{
    const auto before{vector.size()};
    vector.erase(std::remove(vector.begin(), vector.end(), key), vector.end());

    return before - vector.size();
}

template <
    typename Key,
    typename Value,
    typename Out = std::map<Value, Key>,
    typename In = std::map<Key, Value>>
auto reverse_arbitrary_map(const In& map) noexcept -> Out
{
    auto output = Out{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}

template <typename Key, typename Value>
auto reverse_map(const std::map<Key, Value>& map) noexcept
    -> std::map<Value, Key>
{
    return reverse_arbitrary_map<Key, Value>(map);
}
}  // namespace opentxs
