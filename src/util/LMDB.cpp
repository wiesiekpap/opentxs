// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OT_STORAGE_LMDB
#include "util/LMDB.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::storage::lmdb::LMDB::"

namespace opentxs::storage::lmdb
{
LMDB::LMDB(
    const TableNames& names,
    const std::string& folder,
    const TablesToInit init,
    const Flags flags) noexcept
    : names_(names)
    , env_(nullptr)
    , db_()
    , pending_()
    , lock_()
{
    init_environment(folder, init.size(), flags);
    init_tables(init);
}

LMDB::LMDB(LMDB&& rhs) noexcept
    : names_(rhs.names_)
    , env_(rhs.env_)
    , db_(std::move(rhs.db_))
    , pending_(std::move(rhs.pending_))
    , lock_()
{
    rhs.env_ = nullptr;
}

LMDB::Transaction::Transaction(MDB_env* env, const bool rw) noexcept(false)
    : success_(false)
    , ptr_(nullptr)
{
    const Flags flags = rw ? 0u : MDB_RDONLY;

    if (0 != ::mdb_txn_begin(env, nullptr, flags, &ptr_)) {
        throw std::runtime_error("Failed to start transaction");
    }
}

LMDB::Transaction::Transaction(Transaction&& rhs) noexcept
    : success_(rhs.success_)
    , ptr_(rhs.ptr_)
{
    rhs.ptr_ = nullptr;
}

auto LMDB::Transaction::Finalize(const std::optional<bool> success) noexcept
    -> bool
{
    struct Cleanup {
        Cleanup(MDB_txn*& ptr)
            : ptr_(ptr)
        {
        }

        ~Cleanup() { ptr_ = nullptr; }

    private:
        MDB_txn*& ptr_;
    };

    if (nullptr != ptr_) {
        if (success.has_value()) { success_ = success.value(); }

        auto cleanup = Cleanup{ptr_};

        if (success_) {

            return 0 == ::mdb_txn_commit(ptr_);
        } else {
            ::mdb_txn_abort(ptr_);

            return true;
        }
    }

    return false;
}

LMDB::Transaction::~Transaction() { Finalize(); }

auto LMDB::Commit() const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(const LMDB& parent, MDB_txn*& transaction)
            : success_(false)
            , parent_(parent)
            , transaction_(transaction)
        {
        }

        ~Cleanup()
        {
            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }

            parent_.pending_.clear();
        }

    private:
        const LMDB& parent_;
        MDB_txn*& transaction_;
    };

    Lock lock(lock_);
    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, nullptr, 0, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    Cleanup cleanup(*this, transaction);

    for (auto& [table, mode, index, data] : pending_) {
        auto database = db_.at(table);
        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{data.size(), const_cast<char*>(data.data())};
        cleanup.success_ =
            0 == ::mdb_put(transaction, database, &key, &value, 0);

        if (false == cleanup.success_) { break; }
    }

    return cleanup.success_;
}

auto LMDB::Delete(const Table table, MDB_txn* parent) const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction)
            : success_(false)
            , transaction_(transaction)
        {
        }

        ~Cleanup()
        {
            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, parent, 0, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    auto cleanup = Cleanup{transaction};
    const auto database = db_.at(table);
    cleanup.success_ = 0 == ::mdb_drop(transaction, database, 0);

    return cleanup.success_;
}

auto LMDB::Delete(const Table table, const ReadView index, MDB_txn* parent)
    const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction)
            : success_(false)
            , transaction_(transaction)
        {
        }

        ~Cleanup()
        {
            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, parent, 0, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    auto cleanup = Cleanup{transaction};
    const auto database = db_.at(table);
    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    cleanup.success_ = 0 == ::mdb_del(transaction, database, &key, nullptr);

    return cleanup.success_;
}

auto LMDB::Delete(const Table table, const std::size_t index, MDB_txn* parent)
    const noexcept -> bool
{
    return Delete(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        parent);
}

auto LMDB::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data,
    MDB_txn* parent) const noexcept -> bool
{
    return Delete(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        parent);
}

auto LMDB::Delete(
    const Table table,
    const ReadView index,
    const ReadView data,
    MDB_txn* parent) const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction)
            : success_(false)
            , transaction_(transaction)
        {
        }

        ~Cleanup()
        {
            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, parent, 0, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    auto cleanup = Cleanup{transaction};
    const auto database = db_.at(table);
    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{data.size(), const_cast<char*>(data.data())};
    cleanup.success_ = 0 == ::mdb_del(transaction, database, &key, &value);

    return cleanup.success_;
}

auto LMDB::Exists(const Table table, const ReadView index) const noexcept
    -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction, MDB_cursor*& cursor)
            : success_(false)
            , transaction_(transaction)
            , cursor_(cursor)
        {
        }

        ~Cleanup()
        {
            if (nullptr != cursor_) {
                ::mdb_cursor_close(cursor_);
                cursor_ = nullptr;
            }

            if (nullptr != transaction_) {
                ::mdb_txn_abort(transaction_);
                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
        MDB_cursor*& cursor_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, nullptr, MDB_RDONLY, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    MDB_cursor* cursor{nullptr};
    Cleanup cleanup(transaction, cursor);
    const auto database = db_.at(table);

    if (0 != ::mdb_cursor_open(transaction, database, &cursor)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get cursor").Flush();

        return false;
    }

    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{};
    cleanup.success_ = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

    return cleanup.success_;
}

auto LMDB::init_db(const Table table, const std::size_t flags) noexcept
    -> MDB_dbi
{
    MDB_txn* transaction{nullptr};
    auto status = (0 == ::mdb_txn_begin(env_, nullptr, 0, &transaction));

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    auto output = MDB_dbi{};
    status =
        0 ==
        ::mdb_dbi_open(
            transaction, names_.at(table).c_str(), MDB_CREATE | flags, &output);

    OT_ASSERT(status);

    ::mdb_txn_commit(transaction);

    return output;
}

auto LMDB::init_environment(
    const std::string& folder,
    const std::size_t tables,
    const Flags flags) noexcept -> void
{
    bool set = 0 == ::mdb_env_create(&env_);

    OT_ASSERT(set);
    OT_ASSERT(nullptr != env_);

    set = 0 == ::mdb_env_set_mapsize(env_, OT_LMDB_SIZE);

    OT_ASSERT(set);

    set = 0 == ::mdb_env_set_maxdbs(env_, tables);

    OT_ASSERT(set);

    set = 0 == ::mdb_env_set_maxreaders(env_, 1024u);

    OT_ASSERT(set);

    set = 0 == ::mdb_env_open(env_, folder.c_str(), flags, 0664);

    OT_ASSERT(set);
}

auto LMDB::init_tables(const TablesToInit init) noexcept -> void
{
    for (const auto& [table, flags] : init) {
        db_.emplace(table, init_db(table, flags));
    }
}

auto LMDB::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    const Mode multiple) const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction, MDB_cursor*& cursor)
            : success_(false)
            , transaction_(transaction)
            , cursor_(cursor)
        {
        }

        ~Cleanup()
        {
            if (nullptr != cursor_) {
                ::mdb_cursor_close(cursor_);
                cursor_ = nullptr;
            }

            if (nullptr != transaction_) {
                ::mdb_txn_abort(transaction_);
                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
        MDB_cursor*& cursor_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, nullptr, MDB_RDONLY, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    MDB_cursor* cursor{nullptr};
    Cleanup cleanup(transaction, cursor);
    const auto database = db_.at(table);

    if (0 != ::mdb_cursor_open(transaction, database, &cursor)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get cursor").Flush();

        return false;
    }

    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{};
    cleanup.success_ = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

    if (cleanup.success_) {
        ::mdb_cursor_get(cursor, &key, &value, MDB_FIRST_DUP);

        if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
            cb({static_cast<char*>(value.mv_data), value.mv_size});
        } else {

            return false;
        }

        if (static_cast<bool>(multiple)) {
            while (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_NEXT_DUP)) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    cb({static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }
            }
        }
    }

    return cleanup.success_;
}

auto LMDB::Load(
    const Table table,
    const std::size_t index,
    const Callback cb,
    const Mode mode) const noexcept -> bool
{
    return Load(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        cb,
        mode);
}

auto LMDB::Queue(
    const Table table,
    const ReadView key,
    const ReadView value,
    const Mode mode) const noexcept -> bool
{
    Lock lock(lock_);
    pending_.emplace_back(NewKey{table, mode, key, value});

    return true;
}

auto LMDB::Read(const Table table, const ReadCallback cb, const Dir dir)
    const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction, MDB_cursor*& cursor)
            : success_(false)
            , transaction_(transaction)
            , cursor_(cursor)
        {
        }

        ~Cleanup()
        {
            if (nullptr != cursor_) {
                ::mdb_cursor_close(cursor_);
                cursor_ = nullptr;
            }

            if (nullptr != transaction_) {
                ::mdb_txn_abort(transaction_);
                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
        MDB_cursor*& cursor_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, nullptr, MDB_RDONLY, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    MDB_cursor* cursor{nullptr};
    auto cleanup = Cleanup{transaction, cursor};
    const auto database = db_.at(table);

    if (0 != ::mdb_cursor_open(transaction, database, &cursor)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get cursor").Flush();

        return false;
    }

    const auto start =
        MDB_cursor_op{(Dir::Forward == dir) ? MDB_FIRST : MDB_LAST};
    const auto next =
        MDB_cursor_op{(Dir::Forward == dir) ? MDB_NEXT : MDB_PREV};
    auto again{true};
    auto key = MDB_val{};
    auto value = MDB_val{};
    cleanup.success_ = 0 == ::mdb_cursor_get(cursor, &key, &value, start);

    try {
        if (cleanup.success_) {
            if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                again =
                    cb({static_cast<char*>(key.mv_data), key.mv_size},
                       {static_cast<char*>(value.mv_data), value.mv_size});
            } else {

                return false;
            }

            while (again && 0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }
            }
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }

    return cleanup.success_;
}

auto LMDB::ReadFrom(
    const Table table,
    const ReadView index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction, MDB_cursor*& cursor)
            : success_(false)
            , transaction_(transaction)
            , cursor_(cursor)
        {
        }

        ~Cleanup()
        {
            if (nullptr != cursor_) {
                ::mdb_cursor_close(cursor_);
                cursor_ = nullptr;
            }

            if (nullptr != transaction_) {
                ::mdb_txn_abort(transaction_);
                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
        MDB_cursor*& cursor_;
    };

    MDB_txn* transaction{nullptr};

    if (0 != ::mdb_txn_begin(env_, nullptr, MDB_RDONLY, &transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != transaction);

    MDB_cursor* cursor{nullptr};
    auto cleanup = Cleanup{transaction, cursor};
    const auto database = db_.at(table);

    if (0 != ::mdb_cursor_open(transaction, database, &cursor)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get cursor").Flush();

        return false;
    }

    const auto next =
        MDB_cursor_op{(Dir::Forward == dir) ? MDB_NEXT : MDB_PREV};
    auto again{true};
    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{};
    cleanup.success_ = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

    try {
        if (cleanup.success_) {
            if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                again =
                    cb({static_cast<char*>(key.mv_data), key.mv_size},
                       {static_cast<char*>(value.mv_data), value.mv_size});
            } else {

                return false;
            }

            while (again && 0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }
            }
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }

    return cleanup.success_;
}

auto LMDB::ReadFrom(
    const Table table,
    const std::size_t index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    return ReadFrom(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        cb,
        dir);
}

auto LMDB::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction)
            : success_(false)
            , transaction_(transaction)
        {
        }

        ~Cleanup()
        {
            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
    };

    auto output = Result{false, MDB_LAST_ERRCODE};
    auto& [success, code] = output;
    MDB_txn* transaction{nullptr};
    auto cleanup = Cleanup{transaction};
    code = ::mdb_txn_begin(env_, parent, 0, &transaction);

    if (0 != code) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return output;
    }

    OT_ASSERT(nullptr != transaction);

    const auto database = db_.at(table);
    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{data.size(), const_cast<char*>(data.data())};
    code = ::mdb_put(transaction, database, &key, &value, flags);
    success = 0 == code;
    cleanup.success_ = success;

    return output;
}

auto LMDB::Store(
    const Table table,
    const std::size_t index,
    const ReadView data,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    return Store(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        parent,
        flags);
}

auto LMDB::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    struct Cleanup {
        bool success_;

        Cleanup(MDB_txn*& transaction, MDB_cursor*& cursor)
            : success_(false)
            , transaction_(transaction)
            , cursor_(cursor)
        {
        }

        ~Cleanup()
        {
            if (nullptr != cursor_) {
                ::mdb_cursor_close(cursor_);
                cursor_ = nullptr;
            }

            if (nullptr != transaction_) {
                if (success_) {
                    ::mdb_txn_commit(transaction_);
                } else {
                    ::mdb_txn_abort(transaction_);
                }

                transaction_ = nullptr;
            }
        }

    private:
        MDB_txn*& transaction_;
        MDB_cursor*& cursor_;
    };

    auto output = Result{false, MDB_LAST_ERRCODE};

    if (false == bool(cb)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid callback").Flush();

        return output;
    }

    auto& [success, code] = output;
    MDB_cursor* cursor{nullptr};
    MDB_txn* transaction{nullptr};
    auto cleanup = Cleanup{transaction, cursor};
    code = ::mdb_txn_begin(env_, parent, 0, &transaction);

    if (0 != code) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start transaction")
            .Flush();

        return output;
    }

    OT_ASSERT(nullptr != transaction);

    const auto database = db_.at(table);

    if (0 != ::mdb_cursor_open(transaction, database, &cursor)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get cursor").Flush();

        return output;
    }

    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{};
    const auto exists =
        0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET_KEY);
    const auto previous =
        exists
            ? ReadView{static_cast<const char*>(value.mv_data), value.mv_size}
            : ReadView{};

    try {
        const auto bytes = cb(previous);
        auto value =
            MDB_val{bytes.size(), const_cast<std::byte*>(bytes.data())};
        code = ::mdb_put(transaction, database, &key, &value, flags);
        success = 0 == code;
        cleanup.success_ = success;
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
    }

    return output;
}

auto LMDB::TransactionRO() const noexcept(false) -> Transaction
{
    return {env_, false};
}

auto LMDB::TransactionRW() const noexcept(false) -> Transaction
{
    return {env_, true};
}

LMDB::~LMDB()
{
    if (nullptr != env_) {
        ::mdb_env_close(env_);
        env_ = nullptr;
    }
}
}  // namespace opentxs::storage::lmdb
#endif  // OT_STORAGE_LMDB
