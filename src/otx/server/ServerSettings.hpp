// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/util/Container.hpp"

namespace opentxs::server
{
struct ServerSettings {
    static auto GetMinMarketScale() -> std::int64_t
    {
        return _min_market_scale;
    }

    static void SetMinMarketScale(std::int64_t value)
    {
        _min_market_scale = value;
    }

    static auto GetHeartbeatNoRequests() -> std::int32_t
    {
        return _heartbeat_no_requests;
    }

    static void SetHeartbeatNoRequests(std::int32_t value)
    {
        _heartbeat_no_requests = value;
    }

    static auto GetHeartbeatMsBetweenBeats() -> std::int32_t
    {
        return _heartbeat_ms_between_beats;
    }

    static void SetHeartbeatMsBetweenBeats(std::int32_t value)
    {
        _heartbeat_ms_between_beats = value;
    }

    static auto GetOverrideNymID() -> const UnallocatedCString&
    {
        return _override_nym_id;
    }

    static void SetOverrideNymID(const UnallocatedCString& id)
    {
        _override_nym_id = id;
    }

    static std::int64_t _min_market_scale;

    static std::int32_t _heartbeat_no_requests;
    static std::int32_t _heartbeat_ms_between_beats;

    // The Nym who's allowed to do certain commands even if they are turned off.
    static UnallocatedCString _override_nym_id;
    // Are usage credits REQUIRED in order to use this server?
    static bool _admin_usage_credits;
    // Is server currently locked to non-override Nyms?
    static bool _admin_server_locked;

    static bool _cmd_usage_credits;
    static bool _cmd_issue_asset;
    static bool _cmd_get_contract;
    static bool _cmd_check_notary_id;

    static bool _cmd_create_user_acct;
    static bool _cmd_del_user_acct;
    static bool _cmd_check_nym;
    static bool _cmd_get_requestnumber;
    static bool _cmd_get_trans_nums;
    static bool _cmd_send_message;
    static bool _cmd_get_nymbox;
    static bool _cmd_process_nymbox;

    static bool _cmd_create_asset_acct;
    static bool _cmd_del_asset_acct;
    static bool _cmd_get_acct;
    static bool _cmd_get_inbox;
    static bool _cmd_get_outbox;
    static bool _cmd_process_inbox;

    static bool _cmd_issue_basket;
    static bool _transact_exchange_basket;

    static bool _cmd_notarize_transaction;
    static bool _transact_process_inbox;
    static bool _transact_transfer;
    static bool _transact_withdrawal;
    static bool _transact_deposit;
    static bool _transact_withdraw_voucher;
    static bool _transact_deposit_cheque;
    static bool _transact_pay_dividend;

    static bool _cmd_get_mint;
    static bool _transact_withdraw_cash;
    static bool _transact_deposit_cash;

    static bool _cmd_get_market_list;
    static bool _cmd_get_market_offers;
    static bool _cmd_get_market_recent_trades;
    static bool _cmd_get_nym_market_offers;

    static bool _transact_market_offer;
    static bool _transact_payment_plan;
    static bool _transact_cancel_cron_item;
    static bool _transact_smart_contract;
    static bool _cmd_trigger_clause;

    static bool _cmd_register_contract;

    static bool _cmd_request_admin;
};
}  // namespace opentxs::server
