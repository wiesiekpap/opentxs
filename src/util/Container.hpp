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

template <typename Key, typename Value>
auto reverse_map(const std::map<Key, Value>& map) noexcept
    -> std::map<Value, Key>
{
    std::map<Value, Key> output{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}
}  // namespace opentxs
