// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "ui/activitythread/ActivityThreadItem.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QDateTime>
#include <QObject>
#endif  // OT_QT
#include <tuple>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#if OT_QT
#include "opentxs/ui/qt/ActivityThread.hpp"
#endif  // OT_QT
#include "ui/base/Widget.hpp"
#if OT_QT
#include "util/Polarity.hpp"  // IWYU pragma: keep
#endif                        // OT_QT

namespace opentxs::ui::implementation
{
ActivityThreadItem::ActivityThreadItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    const bool loading,
    const bool pending) noexcept
    : ActivityThreadItemRow(parent, api, rowID, true)
    , nym_id_(nymID)
    , time_(std::get<0>(sortKey))
    , item_id_(std::get<0>(row_id_))
    , box_(std::get<1>(row_id_))
    , account_id_(std::get<2>(row_id_))
    , text_(extract_custom<std::string>(custom, 0))
    , loading_(Flag::Factory(loading))
    , pending_(Flag::Factory(pending))
{
    OT_ASSERT(verify_empty(custom));
}

auto ActivityThreadItem::MarkRead() const noexcept -> bool
{
    return api_.Activity().MarkRead(
        nym_id_, Identifier::Factory(parent_.ThreadID()), item_id_);
}

#if OT_QT
QVariant ActivityThreadItem::qt_data(const int column, int role) const noexcept
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case ActivityThreadQt::TimeColumn: {

                    return qt_data(column, ActivityThreadQt::TimeRole);
                }
                case ActivityThreadQt::TextColumn: {

                    return qt_data(column, ActivityThreadQt::TextRole);
                }
                case ActivityThreadQt::AmountColumn: {

                    return qt_data(column, ActivityThreadQt::StringAmountRole);
                }
                case ActivityThreadQt::MemoColumn: {

                    return qt_data(column, ActivityThreadQt::MemoRole);
                }
                case ActivityThreadQt::LoadingColumn:
                case ActivityThreadQt::PendingColumn:
                default: {
                }
            }
        } break;
        case Qt::CheckStateRole: {
            switch (column) {
                case ActivityThreadQt::LoadingColumn: {

                    return qt_data(column, ActivityThreadQt::LoadingRole);
                }
                case ActivityThreadQt::PendingColumn: {

                    return qt_data(column, ActivityThreadQt::PendingRole);
                }
                case ActivityThreadQt::TextColumn:
                case ActivityThreadQt::AmountColumn:
                case ActivityThreadQt::MemoColumn:
                case ActivityThreadQt::TimeColumn:
                default: {
                }
            }
        } break;
        case ActivityThreadQt::IntAmountRole: {

            return static_cast<int>(Amount());
        }
        case ActivityThreadQt::StringAmountRole: {

            return DisplayAmount().c_str();
        }
        case ActivityThreadQt::LoadingRole: {

            return Loading();
        }
        case ActivityThreadQt::MemoRole: {

            return Memo().c_str();
        }
        case ActivityThreadQt::PendingRole: {

            return Pending();
        }
        case ActivityThreadQt::PolarityRole: {

            return polarity(Amount());
        }
        case ActivityThreadQt::TextRole: {

            return Text().c_str();
        }
        case ActivityThreadQt::TimeRole: {
            auto output = QDateTime{};
            output.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            return output;
        }
        case ActivityThreadQt::TypeRole: {

            return static_cast<int>(Type());
        }
        default: {
        }
    }

    return {};
}
#endif

auto ActivityThreadItem::reindex(
    const ActivityThreadSortKey&,
    CustomData& custom) noexcept -> bool
{
    const auto text = extract_custom<std::string>(custom);

    if (false == text.empty()) {
        auto lock = eLock{shared_lock_};
        text_ = text;
    }

    return true;
}

auto ActivityThreadItem::Text() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return text_;
}

auto ActivityThreadItem::Timestamp() const noexcept -> Time
{
    auto lock = sLock{shared_lock_};

    return time_;
}
}  // namespace opentxs::ui::implementation
