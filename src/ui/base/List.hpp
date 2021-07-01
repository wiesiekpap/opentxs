// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <future>
#include <optional>
#include <type_traits>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "internal/core/Core.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/base/Items.hpp"
#include "ui/base/Widget.hpp"

#define LIST_METHOD "opentxs::ui::implementation::List::"

namespace opentxs::ui::implementation
{
template <
    typename ExternalInterface,
    typename InternalInterface,
    typename RowID,
    typename RowInterface,
    typename RowInternal,
    typename RowBlank,
    typename SortKey,
    typename PrimaryID>
class List : virtual public ExternalInterface,
             virtual public InternalInterface,
             public Widget
#if OT_QT
    ,
             public QAbstractItemModel
#endif  // OT_QT
{
#if OT_QT
public:
    using QtPointerType = RowInternal;

    auto columnCount(const QModelIndex& parent) const noexcept -> int override
    {
        if (nullptr == get_pointer(parent)) {
            return column_count_;
        } else {
            return valid_pointers_.columnCount(parent);
        }
    }
    auto data(const QModelIndex& index, int role) const noexcept
        -> QVariant final
    {
        const auto [valid, pRow] = check_index(index);

        if (false == valid) { return {}; }

        return valid_pointers_.data(index, role);
    }
    auto index(int row, int column, const QModelIndex& parent) const noexcept
        -> QModelIndex override
    {
        if (nullptr == get_pointer(parent)) {
            return get_index(row, column);
        } else {
            return valid_pointers_.index(row, column, parent);
        }
    }
    auto parent(const QModelIndex& index) const noexcept -> QModelIndex override
    {
        if (nullptr == get_pointer(index)) {
            return {};
        } else {
            return valid_pointers_.parent(index);
        }
    }
    auto roleNames() const noexcept -> QHash<int, QByteArray> override
    {
        return qt_roles_;
    }
    auto rowCount(const QModelIndex& parent) const noexcept -> int override
    {
        if (nullptr == get_pointer(parent)) {

            return row_count_.load();
        } else {
            return valid_pointers_.rowCount(parent);
        }
    }
    auto register_child(const void* child) const noexcept -> void override
    {
        valid_pointers_.add(child);
    }
    auto unregister_child(const void* child) const noexcept -> void override
    {
        valid_pointers_.remove(child);
    }
#endif  // OT_QT

public:
    using ListPrimaryID = PrimaryID;
#if OT_QT
    using Roles = QHash<int, QByteArray>;
#endif  // OT_QT

    auto First() const noexcept -> SharedPimpl<RowInterface> override
    {
        rLock lock{recursive_lock_};
        counter_ = 0;

        return first(lock);
    }
    virtual auto last(const RowID& id) const noexcept -> bool override
    {
        rLock lock{recursive_lock_};
        const auto index = find_index(id);

        if (0 == items_.size()) { return true; }

        if (index.has_value()) {

            return ((items_.size() - 1) <= index.value());
        } else {

            return true;
        }
    }
    auto Next() const noexcept -> SharedPimpl<RowInterface> override
    {
        rLock lock{recursive_lock_};

        try {

            return SharedPimpl<RowInterface>(
                items_.at(effective_counter(++counter_)).item_);
        } catch (...) {

            return First();
        }
    }

    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        wait_for_startup();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
    auto state_machine() noexcept -> bool { return false; }

    virtual auto WaitForStartup() const noexcept -> void final
    {
        return wait_for_startup();
    }

    ~List() override
    {
        if (startup_ && startup_->joinable()) {
            startup_->join();
            startup_.reset();
        }
    }

protected:
    using RowPointer = std::shared_ptr<RowInternal>;

#if OT_QT
    struct MyPointers {
        auto columnCount(const QModelIndex& parent) const noexcept -> int
        {
            auto lock = Lock{lock_};
            const auto* pointer = get_pointer(parent);

            if ((nullptr != pointer) && exists(lock, pointer)) {

                return pointer->qt_column_count();
            }

            return {};
        }

        auto data(const QModelIndex& index, int role) const noexcept -> QVariant
        {
            auto lock = Lock{lock_};
            const auto* pointer = get_pointer(index);

            if ((nullptr != pointer) && exists(lock, pointer)) {

                return pointer->qt_data(index.column(), role);
            }

            return {};
        }
        auto index(int row, int column, const QModelIndex& parent)
            const noexcept -> QModelIndex
        {
            auto lock = Lock{lock_};
            const auto* pointer = get_pointer(parent);

            if ((nullptr != pointer) && exists(lock, pointer)) {

                return pointer->qt_index(row, column);
            }

            return {};
        }
        auto parent(const QModelIndex& index) const noexcept -> QModelIndex
        {
            auto lock = Lock{lock_};
            const auto* pointer = get_pointer(index);

            if ((nullptr != pointer) && exists(lock, pointer)) {

                return pointer->qt_parent();
            }

            return {};
        }
        auto rowCount(const QModelIndex& parent) const noexcept -> int
        {
            auto lock = Lock{lock_};
            const auto* pointer = get_pointer(parent);

            if ((nullptr != pointer) && exists(lock, pointer)) {

                return pointer->qt_row_count();
            }

            return {};
        }

        auto add(const void* child) const noexcept -> void
        {
            auto lock = Lock{lock_};
            valid_.insert(reinterpret_cast<std::uintptr_t>(child));
        }
        auto remove(const void* child) const noexcept -> void
        {
            auto lock = Lock{lock_};
            valid_.erase(reinterpret_cast<std::uintptr_t>(child));
        }

    private:
        mutable std::mutex lock_{};
        mutable std::set<std::uintptr_t> valid_{};

        auto exists(const Lock&, const void* p) const noexcept -> bool
        {
            return 0 < valid_.count(reinterpret_cast<std::uintptr_t>(p));
        }
    };
#endif  // OT_QT

    mutable std::recursive_mutex recursive_lock_;
    mutable std::shared_mutex shared_lock_;
#if OT_QT
    mutable std::atomic<int> row_count_;
    const int column_count_;
    MyPointers valid_pointers_;
#endif  // OT_QT
    const PrimaryID primary_id_;
    mutable std::size_t counter_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    std::unique_ptr<std::thread> startup_;

#if OT_QT
    static auto get_pointer(const QModelIndex& index) -> const QtPointerType*
    {
        return static_cast<const QtPointerType*>(index.internalPointer());
    }

    auto check_index(const QModelIndex& index) const noexcept
        -> std::pair<bool, const QtPointerType*>
    {
        std::pair<bool, const QtPointerType*> output{false, nullptr};
        auto& [valid, row] = output;

        if (false == index.isValid()) { return output; }

        if (column_count_ < index.column()) { return output; }

        if (nullptr == index.internalPointer()) { return output; }

        row = get_pointer(index);
        valid = (nullptr != row);

        return output;
    }
#endif  // OT_QT
    auto delete_inactive(const std::set<RowID>& active) const noexcept -> void
    {
        rLock lock{recursive_lock_};
        delete_inactive(lock, active);
    }
    auto delete_inactive(const rLock& lock, const std::set<RowID>& active)
        const noexcept -> void
    {
        const auto existing = items_.active();
        auto deleteIDs = std::vector<RowID>{};
        std::set_difference(
            existing.begin(),
            existing.end(),
            active.begin(),
            active.end(),
            std::back_inserter(deleteIDs));

        for (const auto& id : deleteIDs) { delete_item(lock, id); }

        if (0 < deleteIDs.size()) { UpdateNotify(); }
    }
    auto delete_item(const RowID& id) const noexcept -> void
    {
        rLock lock{recursive_lock_};
        delete_item(lock, id);
    }
    auto delete_item(const rLock&, const RowID& id) const noexcept -> void
    {
        auto position = items_.find_delete_position(id);

        if (false == position.has_value()) { return; }

        auto& [it, index] = position.value();
#if OT_QT
        const auto row = static_cast<int>(index);
        emit_begin_remove_rows(me(), row, row);
        unregister_child(it->item_.get());
#endif  // OT_QT
        items_.delete_row(id, it);
#if OT_QT
        --row_count_;
        emit_end_remove_rows();
#endif  // OT_QT
    }
    virtual auto default_id() const noexcept -> RowID
    {
        return make_blank<RowID>::value(api_);
    }
    virtual auto effective_counter(const std::size_t value) const noexcept
        -> std::size_t
    {
        return value;
    }
    virtual auto find_index(const RowID& id) const noexcept
        -> std::optional<std::size_t>
    {
        return items_.get_index(id);
    }
#if OT_QT
    virtual auto find_row(const RowID& id) const noexcept -> int
    {
        if (const auto index{find_index(id)}; index.has_value()) {

            return static_cast<int>(index.value());
        } else {

            return -1;
        }
    }
#endif  // OT_QT
    virtual auto first(const rLock&) const noexcept -> SharedPimpl<RowInterface>
    {
        try {

            return SharedPimpl<RowInterface>(items_.at(0).item_);
        } catch (...) {

            return SharedPimpl<RowInterface>(blank_p_);
        }
    }
#if OT_QT
    auto get_index(const int row, const int column) const noexcept
        -> QModelIndex
    {
        rLock lock{recursive_lock_};

        return get_index(lock, row, column);
    }
#endif  // OT_QT
    virtual auto lookup(const rLock&, const RowID& id) const noexcept
        -> const RowInternal&
    {
        try {

            return *(items_.get(id).item_);
        } catch (...) {

            return blank_;
        }
    }
    auto size() const noexcept -> std::size_t { return items_.size(); }
    auto wait_for_startup() const noexcept -> void { startup_future_.get(); }

    virtual auto add_item(
        const RowID& id,
        const SortKey& index,
        CustomData& custom) noexcept -> void
    {
        rLock lock{recursive_lock_};
        auto existing = find_index(id);

        if (existing.has_value()) {
            move_item(lock, id, index, custom);
        } else {
            insert_item(lock, id, index, custom);
        }
    }
    auto finish_startup() noexcept -> void
    {
        try {
            startup_promise_.set_value();
        } catch (...) {
        }
    }
    auto init() noexcept -> void {}
    auto row_modified(const RowID& id) noexcept -> void
    {
        rLock lock{recursive_lock_};
        row_modified(id);
    }
    virtual auto row_modified(const Lock&, const RowID& id) noexcept -> void
    {
        const auto lookup = find_index(id);

        if (false == lookup.has_value()) { return; }

        const auto index = lookup.value();
        auto& row = items_.at(index);
        row_modified(index, row.item_.get());
    }
    // FIXME The Combined child class must override this
    virtual auto row_modified(
        [[maybe_unused]] const std::size_t index,
        [[maybe_unused]] RowInternal* pointer) noexcept -> void
    {
#if OT_QT
        const auto row = static_cast<int>(index);
        emit_data_changed(
            createIndex(row, 0, pointer),
            createIndex(row, column_count_, pointer));
#endif  // OT_QT
        UpdateNotify();
    }

    List(
        const api::client::internal::Manager& api,
        const typename PrimaryID::interface_type& primaryID,
        const Identifier& widgetID,
        const bool reverseSort = false,
#if OT_QT
        const Roles& roles = {},
        const int columns = 0,
        const int startRow = 0,
#endif  // OT_QT
        const bool subnode = true,
        const SimpleCallback& cb = {}) noexcept
        : Widget(api, widgetID, cb)
        , recursive_lock_()
        , shared_lock_()
#if OT_QT
        , row_count_(startRow)
        , column_count_(columns)
        , valid_pointers_()
#endif  // OT_QT
        , primary_id_(primaryID)
        , counter_(0)
        , have_items_(Flag::Factory(false))
        , start_(Flag::Factory(true))
        , startup_(nullptr)
#if OT_QT
        , qt_roles_(roles)
#endif  // OT_QT
        , blank_p_(std::make_unique<RowBlank>())
        , blank_(*blank_p_)
        , subnode_(subnode)
        , init_(false)
        , items_(reverseSort)
        , startup_promise_()
        , startup_future_(startup_promise_.get_future())
    {
        OT_ASSERT(blank_p_);
        OT_ASSERT(!(subnode && bool(cb)));
    }
    List(
        const api::client::internal::Manager& api,
        const typename PrimaryID::interface_type& primaryID,
        const SimpleCallback& cb,
        const bool reverseSort = false
#if OT_QT
        ,
        const Roles& roles = {},
        const int columns = 0,
        const int startRow = 0
#endif  // OT_QT
        ) noexcept
        : List(
              api,
              primaryID,
              Identifier::Random(),
              reverseSort,
#if OT_QT
              roles,
              columns,
              startRow,
#endif  // OT_QT
              false,
              cb)
    {
    }

private:
    using ItemsType = ListItems<RowID, SortKey, RowPointer>;

#if OT_QT
    const Roles qt_roles_;
#endif  // OT_QT
    const std::shared_ptr<const RowInternal> blank_p_;
    const RowInternal& blank_;
    const bool subnode_;
    mutable std::atomic<bool> init_;
    mutable ItemsType items_;
    std::promise<void> startup_promise_;
    std::shared_future<void> startup_future_;

    virtual auto construct_row(
        const RowID& id,
        const SortKey& index,
        CustomData& custom) const noexcept -> RowPointer = 0;
#if OT_QT
    auto emit_begin_insert_rows(const QModelIndex& parent, int first, int last)
        const noexcept -> void override
    {
        const_cast<List&>(*this).beginInsertRows(parent, first, last);
    }
    auto emit_begin_move_rows(
        const QModelIndex& source,
        int start,
        int end,
        const QModelIndex& destination,
        int to) const noexcept -> void override
    {
        const_cast<List&>(*this).beginMoveRows(
            source, start, end, destination, to);
    }
    auto emit_begin_remove_rows(const QModelIndex& parent, int first, int last)
        const noexcept -> void override
    {
        const_cast<List&>(*this).beginRemoveRows(parent, first, last);
    }
    auto emit_data_changed(
        const QModelIndex& topLeft,
        const QModelIndex& bottomRight) noexcept
        -> void override  // FIXME Combined must override this
    {
        emit dataChanged(topLeft, bottomRight);
    }
    auto emit_end_insert_rows() const noexcept -> void override
    {
        const_cast<List&>(*this).endInsertRows();
    }
    auto emit_end_move_rows() const noexcept -> void override
    {
        const_cast<List&>(*this).endMoveRows();
    }
    auto emit_end_remove_rows() const noexcept -> void override
    {
        const_cast<List&>(*this).endRemoveRows();
    }
    auto get_index(const rLock&, const int row, const int column) const noexcept
        -> QModelIndex
    {
        if (column_count_ < column) { return {}; }
        if (0 > row) { return {}; }

        try {
            auto data = items_.at(static_cast<std::size_t>(row));
            QtPointerType* item{data.item_.get()};

            if (nullptr == item) { return {}; }

            return createIndex(row, column, item);
        } catch (...) {

            return {};
        }
    }
#endif  // OT_QT
    auto insert_item(
        const rLock&,
        const RowID& id,
        const SortKey& key,
        CustomData& custom) noexcept -> void
    {
        auto pointer = construct_row(id, key, custom);

        if (false == bool(pointer)) { return; }

        const auto position = items_.find_insert_position(key, id);
        auto& [it, index] = position;
#if OT_QT
        const auto row = static_cast<int>(index);
        emit_begin_insert_rows(me(), row, row);
#endif  // OT_QT
        items_.insert_before(it, key, id, pointer);
#if OT_QT
        register_child(pointer.get());
        ++row_count_;
        emit_end_insert_rows();
#endif  // OT_QT
        UpdateNotify();
    }
#if OT_QT
    auto me() const noexcept -> QModelIndex override { return {}; }
#endif  // OT_QT
    auto move_item(
        const rLock&,
        const RowID& id,
        const SortKey& key,
        CustomData& custom) noexcept -> void
    {
        auto move = items_.find_move_position(id, key, id);

        if (false == move.has_value()) { return; }

        auto& [from, to] = move.value();
        auto& [source, fromRow] = from;
        auto& [dest, toRow] = to;
        auto item = source->item_.get();
        const auto samePosition{(fromRow == toRow) || ((fromRow + 1) == toRow)};

        OT_ASSERT(std::numeric_limits<int>::max() >= fromRow);
        OT_ASSERT(std::numeric_limits<int>::max() >= toRow);

#if OT_QT
        if (false == samePosition) {
            emit_begin_move_rows(
                me(),
                static_cast<int>(fromRow),
                static_cast<int>(fromRow),
                me(),
                static_cast<int>(toRow));
        }
#endif  // OT_QT

        items_.move_before(id, source, key, id, dest);
        const auto changed = item->reindex(key, custom);

#if OT_QT
        if (samePosition) {
            if (changed) {
                emit_data_changed(
                    createIndex(static_cast<int>(fromRow), 0, item),
                    createIndex(
                        static_cast<int>(fromRow), column_count_, item));
            }
        } else {
            emit_end_move_rows();
        }
#endif  // OT_QT

        if (changed || (!samePosition)) { UpdateNotify(); }
    }

    List() = delete;
    List(const List&) = delete;
    List(List&&) = delete;
    auto operator=(const List&) -> List& = delete;
    auto operator=(List&&) -> List& = delete;
};
}  // namespace opentxs::ui::implementation
