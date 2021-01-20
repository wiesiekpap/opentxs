// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"

namespace opentxs::ui::implementation
{
template <typename ListTemplate, typename RowTemplate, typename SortKey>
class Combined : public ListTemplate, public RowTemplate
{
public:
#if OT_QT
    auto emit_begin_insert_rows(const QModelIndex& parent, int first, int last)
        const noexcept -> void final
    {
        this->parent_.emit_begin_insert_rows(parent, first, last);
    }
    auto emit_begin_move_rows(
        const QModelIndex& source,
        int start,
        int end,
        const QModelIndex& dest,
        int to) const noexcept -> void final
    {
        this->parent_.emit_begin_move_rows(source, start, end, dest, to);
    }
    auto emit_begin_remove_rows(const QModelIndex& parent, int first, int last)
        const noexcept -> void final
    {
        this->parent_.emit_begin_remove_rows(parent, first, last);
    }
    auto emit_end_insert_rows() const noexcept -> void final
    {
        this->parent_.emit_end_insert_rows();
    }
    auto emit_end_move_rows() const noexcept -> void final
    {
        this->parent_.emit_end_move_rows();
    }
    auto emit_end_remove_rows() const noexcept -> void final
    {
        this->parent_.emit_end_remove_rows();
    }
    auto me() const noexcept -> QModelIndex final
    {
        sLock lock(this->shared_lock_);

        return QAbstractItemModel::createIndex(
            this->parent_.FindRow(this->row_id_),
            0,
            static_cast<typename ListTemplate::QtPointerType*>(
                const_cast<Combined*>(this)));
    }
    int qt_column_count() const noexcept final { return this->column_count_; }
    QModelIndex qt_index(const int row, const int column) const noexcept final
    {
        return this->get_index(row, column);
    }
    int qt_row_count() const noexcept final { return this->row_count_.load(); }
    void register_child(const void* child) const noexcept
    {
        this->parent_.register_child(child);
        this->valid_pointers_.add(child);
    }
    void unregister_child(const void* child) const noexcept
    {
        this->parent_.unregister_child(child);
        this->valid_pointers_.remove(child);
    }
#endif

protected:
    SortKey key_;

    Combined(
        const api::client::internal::Manager& api,
        const typename ListTemplate::ListPrimaryID::interface_type& primaryID,
        const Identifier& widgetID,
        const typename RowTemplate::RowParentType& parent,
        const typename RowTemplate::RowIdentifierType id,
        const SortKey key,
        const bool reverseSort = false
#if OT_QT
        ,
        const typename ListTemplate::Roles& roles = {},
        const int columns = 0,
        const int startRow = 0
#endif
        ) noexcept
        : ListTemplate(
              api,
              primaryID,
              widgetID,
              reverseSort
#if OT_QT
              ,
              roles,
              columns,
              startRow
#endif
              )
        , RowTemplate(parent, id, true)
        , key_(key)
    {
    }
    Combined() = delete;
    Combined(const Combined&) = delete;
    Combined(Combined&&) = delete;
    auto operator=(const Combined&) -> Combined& = delete;
    auto operator=(Combined&&) -> Combined& = delete;

    virtual ~Combined() = default;
};
}  // namespace opentxs::ui::implementation
