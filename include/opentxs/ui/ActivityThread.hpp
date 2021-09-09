// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYTHREAD_HPP
#define OPENTXS_UI_ACTIVITYTHREAD_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

namespace opentxs
{
namespace ui
{
class ActivityThread;
class ActivityThreadItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ActivityThread : virtual public List
{
public:
    virtual auto CanMessage() const noexcept -> bool = 0;
    virtual auto DisplayName() const noexcept -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> = 0;
    virtual auto GetDraft() const noexcept -> std::string = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> = 0;
    virtual auto Participants() const noexcept -> std::string = 0;
    virtual auto Pay(
        const std::string& amount,
        const Identifier& sourceAccount,
        const std::string& memo = "",
        const PaymentType type = PaymentType::Cheque) const noexcept
        -> bool = 0;
    virtual auto Pay(
        const Amount amount,
        const Identifier& sourceAccount,
        const std::string& memo = "",
        const PaymentType type = PaymentType::Cheque) const noexcept
        -> bool = 0;
    virtual auto PaymentCode(const contact::ContactItemType currency)
        const noexcept -> std::string = 0;
    virtual auto SendDraft() const noexcept -> bool = 0;
    virtual auto SetDraft(const std::string& draft) const noexcept -> bool = 0;
    virtual auto ThreadID() const noexcept -> std::string = 0;

    ~ActivityThread() override = default;

protected:
    ActivityThread() noexcept = default;

private:
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    auto operator=(const ActivityThread&) -> ActivityThread& = delete;
    auto operator=(ActivityThread&&) -> ActivityThread& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
