// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <optional>

namespace opentxs::ui::implementation
{
template <typename RowID, typename SortKey, typename RowPointer>
class ListItems
{
public:
    struct Row {
        SortKey key_;
        RowID id_;
        RowPointer item_;
    };
    using Data = std::list<Row>;
    using Iterator = typename Data::iterator;
    using Index = std::map<RowID, Iterator>;
    using Position = std::pair<Iterator, std::size_t>;
    using Move = std::pair<Position, Position>;

    auto active() const noexcept -> std::vector<RowID>
    {
        auto output = std::vector<RowID>{};
        output.reserve(index_.size());
        std::transform(
            index_.begin(), index_.end(), std::back_inserter(output), [
            ](const auto& in) -> auto { return in.first; });

        return output;
    }
    auto size() const noexcept { return data_.size(); }

    auto at(const std::size_t pos) -> Row&
    {
        if ((0 == data_.size()) || ((data_.size() - 1) < pos)) {
            throw std::out_of_range("Invalid position");
        }

        auto output = data_.begin();
        std::advance(output, pos);

        return *output;
    }
    auto get(const RowID& id) -> Row& { return *index_.at(id); }
    auto begin() noexcept -> Iterator { return data_.begin(); }
    auto delete_row(const RowID& id, Iterator position) noexcept -> void
    {
        data_.erase(position);
        index_.erase(id);
    }
    auto end() noexcept -> Iterator { return data_.end(); }
    auto find_delete_position(const RowID& id) noexcept
        -> std::optional<Position>
    {
        try {
            const auto it = index_.at(id);

            return Position{
                it, static_cast<std::size_t>(std::distance(data_.begin(), it))};

        } catch (...) {

            return std::nullopt;
        }
    }
    auto find_insert_position(const SortKey& key, const RowID& id) noexcept
        -> Position
    {
        auto output = Position{data_.end(), 0};
        auto& [it, index] = output;

        for (auto i{data_.begin()}; i != data_.end(); ++i, ++index) {
            const auto& [rKey, rId, item] = *i;

            if (sort(key, id, rKey, rId)) { continue; }

            it = i;

            break;
        }

        return output;
    }
    auto find_move_position(
        const RowID& oldId,
        const SortKey& newKey,
        const RowID& newID) noexcept -> std::optional<Move>
    {
        auto output = Move{{data_.end(), 0}, {data_.end(), 0}};
        auto& [from, to] = output;
        auto& [it, index] = to;

        try {
            const auto it = index_.at(oldId);
            from = Position{
                it, static_cast<std::size_t>(std::distance(data_.begin(), it))};
        } catch (...) {

            return std::nullopt;
        }

        for (auto i{data_.begin()}; i != data_.end(); ++i, ++index) {
            const auto& [rKey, rId, item] = *i;

            if (sort(newKey, newID, rKey, rId)) { continue; }

            it = i;

            break;
        }

        return std::move(output);
    }
    auto get_index(const RowID& id) noexcept -> std::optional<std::size_t>
    {
        try {

            return static_cast<std::size_t>(
                std::distance(data_.begin(), index_.at(id)));
        } catch (...) {

            return std::nullopt;
        }
    }
    auto insert_before(
        const Iterator& position,
        const SortKey& key,
        const RowID& id,
        const RowPointer& item) noexcept
    {
        auto& index = index_[id];
        index = data_.emplace(position, Row{key, id, item});
    }
    auto move_before(
        const RowID& oldId,
        Iterator oldPosition,
        const SortKey& newKey,
        const RowID& newID,
        Iterator newPosition) noexcept -> void
    {
        index_.erase(oldId);
        auto& oldData = *oldPosition;
        auto& index = index_[newID];
        index = data_.emplace(newPosition, Row{newKey, newID, oldData.item_});
        data_.erase(oldPosition);
    }

    ListItems(const bool reverse) noexcept
        : reverse_sort_(reverse)
        , data_()
        , index_()
    {
    }

private:
    const bool reverse_sort_;
    Data data_;
    Index index_;

    template <typename T>
    auto sort(const T& lhs, const T& rhs) const noexcept -> bool
    {
        static const auto compare = std::less<T>{};

        if (reverse_sort_) {

            return compare(rhs, lhs);
        } else {

            return compare(lhs, rhs);
        }
    }
    auto sort(
        const SortKey& incomingKey,
        const RowID& incomingID,
        const SortKey& existingKey,
        const RowID& existingID) const noexcept -> bool
    {
        if (sort(existingKey, incomingKey)) { return true; }

        return (existingKey == incomingKey) && sort(existingID, incomingID);
    }
};
}  // namespace opentxs::ui::implementation
