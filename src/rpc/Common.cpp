// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "internal/rpc/RPC.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <functional>

#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/rpc/AccountEventType.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/ContactEventType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/PushType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "util/Container.hpp"

namespace opentxs::rpc
{
using AccountEventMap =
    boost::container::flat_map<AccountEventType, proto::AccountEventType>;
using AccountEventReverseMap =
    boost::container::flat_map<proto::AccountEventType, AccountEventType>;
using AccountMap = boost::container::flat_map<AccountType, proto::AccountType>;
using AccountReverseMap =
    boost::container::flat_map<proto::AccountType, AccountType>;
using CommandMap =
    boost::container::flat_map<CommandType, proto::RPCCommandType>;
using CommandReverseMap =
    boost::container::flat_map<proto::RPCCommandType, CommandType>;
using ContactEventMap =
    boost::container::flat_map<ContactEventType, proto::ContactEventType>;
using ContactEventReverseMap =
    boost::container::flat_map<proto::ContactEventType, ContactEventType>;
using PaymentMap =
    boost::container::flat_map<PaymentType, proto::RPCPaymentType>;
using PaymentReverseMap =
    boost::container::flat_map<proto::RPCPaymentType, PaymentType>;
using PushMap = boost::container::flat_map<PushType, proto::RPCPushType>;
using PushReverseMap = boost::container::flat_map<proto::RPCPushType, PushType>;
using ResponseCodeMap =
    boost::container::flat_map<ResponseCode, proto::RPCResponseCode>;
using ResponseCodeReverseMap =
    boost::container::flat_map<proto::RPCResponseCode, ResponseCode>;

auto account_event_map() noexcept -> AccountEventMap;
auto account_event_map() noexcept -> AccountEventMap
{
    static const auto map = AccountEventMap{
        {AccountEventType::error, proto::ACCOUNTEVENT_ERROR},
        {AccountEventType::incoming_cheque, proto::ACCOUNTEVENT_INCOMINGCHEQUE},
        {AccountEventType::outgoing_cheque, proto::ACCOUNTEVENT_OUTGOINGCHEQUE},
        {AccountEventType::incoming_transfer,
         proto::ACCOUNTEVENT_INCOMINGTRANSFER},
        {AccountEventType::outgoing_transfer,
         proto::ACCOUNTEVENT_OUTGOINGTRANSFER},
        {AccountEventType::incoming_invoice,
         proto::ACCOUNTEVENT_INCOMINGINVOICE},
        {AccountEventType::outgoing_invoice,
         proto::ACCOUNTEVENT_OUTGOINGINVOICE},
        {AccountEventType::incoming_voucher,
         proto::ACCOUNTEVENT_INCOMINGVOUCHER},
        {AccountEventType::outgoing_voucher,
         proto::ACCOUNTEVENT_OUTGOINGVOUCHER},
        {AccountEventType::incoming_blockchain,
         proto::ACCOUNTEVENT_INCOMINGBLOCKCHAIN},
        {AccountEventType::outgoing_blockchain,
         proto::ACCOUNTEVENT_OUTGOINGBLOCKCHAIN},
    };

    return map;
}
auto account_map() noexcept -> AccountMap;
auto account_map() noexcept -> AccountMap
{
    static const auto map = AccountMap{
        {AccountType::error, proto::ACCOUNTTYPE_ERROR},
        {AccountType::normal, proto::ACCOUNTTYPE_NORMAL},
        {AccountType::issuer, proto::ACCOUNTTYPE_ISSUER},
        {AccountType::blockchain, proto::ACCOUNTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto command_map() noexcept -> CommandMap;
auto command_map() noexcept -> CommandMap
{
    static const auto map = CommandMap{
        {CommandType::error, proto::RPCCOMMAND_ERROR},
        {CommandType::add_client_session, proto::RPCCOMMAND_ADDCLIENTSESSION},
        {CommandType::add_server_session, proto::RPCCOMMAND_ADDSERVERSESSION},
        {CommandType::list_client_sessions,
         proto::RPCCOMMAND_LISTCLIENTSESSIONS},
        {CommandType::list_server_sessions,
         proto::RPCCOMMAND_LISTSERVERSESSIONS},
        {CommandType::import_hd_seed, proto::RPCCOMMAND_IMPORTHDSEED},
        {CommandType::list_hd_seeds, proto::RPCCOMMAND_LISTHDSEEDS},
        {CommandType::get_hd_seed, proto::RPCCOMMAND_GETHDSEED},
        {CommandType::create_nym, proto::RPCCOMMAND_CREATENYM},
        {CommandType::list_nyms, proto::RPCCOMMAND_LISTNYMS},
        {CommandType::get_nym, proto::RPCCOMMAND_GETNYM},
        {CommandType::add_claim, proto::RPCCOMMAND_ADDCLAIM},
        {CommandType::delete_claim, proto::RPCCOMMAND_DELETECLAIM},
        {CommandType::import_server_contract,
         proto::RPCCOMMAND_IMPORTSERVERCONTRACT},
        {CommandType::list_server_contracts,
         proto::RPCCOMMAND_LISTSERVERCONTRACTS},
        {CommandType::register_nym, proto::RPCCOMMAND_REGISTERNYM},
        {CommandType::create_unit_definition,
         proto::RPCCOMMAND_CREATEUNITDEFINITION},
        {CommandType::list_unit_definitions,
         proto::RPCCOMMAND_LISTUNITDEFINITIONS},
        {CommandType::issue_unit_definition,
         proto::RPCCOMMAND_ISSUEUNITDEFINITION},
        {CommandType::create_account, proto::RPCCOMMAND_CREATEACCOUNT},
        {CommandType::list_accounts, proto::RPCCOMMAND_LISTACCOUNTS},
        {CommandType::get_account_balance, proto::RPCCOMMAND_GETACCOUNTBALANCE},
        {CommandType::get_account_activity,
         proto::RPCCOMMAND_GETACCOUNTACTIVITY},
        {CommandType::send_payment, proto::RPCCOMMAND_SENDPAYMENT},
        {CommandType::move_funds, proto::RPCCOMMAND_MOVEFUNDS},
        {CommandType::add_contact, proto::RPCCOMMAND_ADDCONTACT},
        {CommandType::list_contacts, proto::RPCCOMMAND_LISTCONTACTS},
        {CommandType::get_contact, proto::RPCCOMMAND_GETCONTACT},
        {CommandType::add_contact_claim, proto::RPCCOMMAND_ADDCONTACTCLAIM},
        {CommandType::delete_contact_claim,
         proto::RPCCOMMAND_DELETECONTACTCLAIM},
        {CommandType::verify_claim, proto::RPCCOMMAND_VERIFYCLAIM},
        {CommandType::accept_verification,
         proto::RPCCOMMAND_ACCEPTVERIFICATION},
        {CommandType::send_contact_message,
         proto::RPCCOMMAND_SENDCONTACTMESSAGE},
        {CommandType::get_contact_activity,
         proto::RPCCOMMAND_GETCONTACTACTIVITY},
        {CommandType::get_server_contract, proto::RPCCOMMAND_GETSERVERCONTRACT},
        {CommandType::get_pending_payments,
         proto::RPCCOMMAND_GETPENDINGPAYMENTS},
        {CommandType::accept_pending_payments,
         proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS},
        {CommandType::get_compatible_accounts,
         proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS},
        {CommandType::create_compatible_account,
         proto::RPCCOMMAND_CREATECOMPATIBLEACCOUNT},
        {CommandType::get_workflow, proto::RPCCOMMAND_GETWORKFLOW},
        {CommandType::get_server_password, proto::RPCCOMMAND_GETSERVERPASSWORD},
        {CommandType::get_admin_nym, proto::RPCCOMMAND_GETADMINNYM},
        {CommandType::get_unit_definition, proto::RPCCOMMAND_GETUNITDEFINITION},
        {CommandType::get_transaction_data,
         proto::RPCCOMMAND_GETTRANSACTIONDATA},
        {CommandType::lookup_accountid, proto::RPCCOMMAND_LOOKUPACCOUNTID},
        {CommandType::rename_account, proto::RPCCOMMAND_RENAMEACCOUNT},
    };

    return map;
}
auto contact_event_map() noexcept -> ContactEventMap;
auto contact_event_map() noexcept -> ContactEventMap
{
    static const auto map = ContactEventMap{
        {ContactEventType::error, proto::CONTACTEVENT_ERROR},
        {ContactEventType::incoming_message,
         proto::CONTACTEVENT_INCOMINGMESSAGE},
        {ContactEventType::outgoing_message,
         proto::CONTACTEVENT_OUTGOINGMESSAGE},
        {ContactEventType::incomong_payment,
         proto::CONTACTEVENT_INCOMONGPAYMENT},
        {ContactEventType::outgoing_payment,
         proto::CONTACTEVENT_OUTGOINGPAYMENT},
    };

    return map;
}
auto payment_map() noexcept -> PaymentMap;
auto payment_map() noexcept -> PaymentMap
{
    static const auto map = PaymentMap{
        {PaymentType::error, proto::RPCPAYMENTTYPE_ERROR},
        {PaymentType::cheque, proto::RPCPAYMENTTYPE_CHEQUE},
        {PaymentType::transfer, proto::RPCPAYMENTTYPE_TRANSFER},
        {PaymentType::voucher, proto::RPCPAYMENTTYPE_VOUCHER},
        {PaymentType::invoice, proto::RPCPAYMENTTYPE_INVOICE},
        {PaymentType::blinded, proto::RPCPAYMENTTYPE_BLINDED},
        {PaymentType::blockchain, proto::RPCPAYMENTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto push_map() noexcept -> PushMap;
auto push_map() noexcept -> PushMap
{
    static const auto map = PushMap{
        {PushType::error, proto::RPCPUSH_ERROR},
        {PushType::account, proto::RPCPUSH_ACCOUNT},
        {PushType::contact, proto::RPCPUSH_CONTACT},
        {PushType::task, proto::RPCPUSH_TASK},
    };

    return map;
}
auto response_code_map() noexcept -> ResponseCodeMap;
auto response_code_map() noexcept -> ResponseCodeMap
{
    static const auto map = ResponseCodeMap{
        {ResponseCode::invalid, proto::RPCRESPONSE_INVALID},
        {ResponseCode::success, proto::RPCRESPONSE_SUCCESS},
        {ResponseCode::bad_session, proto::RPCRESPONSE_BAD_SESSION},
        {ResponseCode::none, proto::RPCRESPONSE_NONE},
        {ResponseCode::queued, proto::RPCRESPONSE_QUEUED},
        {ResponseCode::unnecessary, proto::RPCRESPONSE_UNNECESSARY},
        {ResponseCode::retry, proto::RPCRESPONSE_RETRY},
        {ResponseCode::no_path_to_recipient,
         proto::RPCRESPONSE_NO_PATH_TO_RECIPIENT},
        {ResponseCode::bad_server_argument,
         proto::RPCRESPONSE_BAD_SERVER_ARGUMENT},
        {ResponseCode::cheque_not_found, proto::RPCRESPONSE_CHEQUE_NOT_FOUND},
        {ResponseCode::payment_not_found, proto::RPCRESPONSE_PAYMENT_NOT_FOUND},
        {ResponseCode::start_task_failed, proto::RPCRESPONSE_START_TASK_FAILED},
        {ResponseCode::nym_not_found, proto::RPCRESPONSE_NYM_NOT_FOUND},
        {ResponseCode::add_claim_failed, proto::RPCRESPONSE_ADD_CLAIM_FAILED},
        {ResponseCode::add_contact_failed,
         proto::RPCRESPONSE_ADD_CONTACT_FAILED},
        {ResponseCode::register_account_failed,
         proto::RPCRESPONSE_REGISTER_ACCOUNT_FAILED},
        {ResponseCode::bad_server_response,
         proto::RPCRESPONSE_BAD_SERVER_RESPONSE},
        {ResponseCode::workflow_not_found,
         proto::RPCRESPONSE_WORKFLOW_NOT_FOUND},
        {ResponseCode::unit_definition_not_found,
         proto::RPCRESPONSE_UNITDEFINITION_NOT_FOUND},
        {ResponseCode::session_not_found, proto::RPCRESPONSE_SESSION_NOT_FOUND},
        {ResponseCode::create_nym_failed, proto::RPCRESPONSE_CREATE_NYM_FAILED},
        {ResponseCode::create_unit_definition_failed,
         proto::RPCRESPONSE_CREATE_UNITDEFINITION_FAILED},
        {ResponseCode::delete_claim_failed,
         proto::RPCRESPONSE_DELETE_CLAIM_FAILED},
        {ResponseCode::account_not_found, proto::RPCRESPONSE_ACCOUNT_NOT_FOUND},
        {ResponseCode::move_funds_failed, proto::RPCRESPONSE_MOVE_FUNDS_FAILED},
        {ResponseCode::register_nym_failed,
         proto::RPCRESPONSE_REGISTER_NYM_FAILED},
        {ResponseCode::contact_not_found, proto::RPCRESPONSE_CONTACT_NOT_FOUND},
        {ResponseCode::account_owner_not_found,
         proto::RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND},
        {ResponseCode::send_payment_failed,
         proto::RPCRESPONSE_SEND_PAYMENT_FAILED},
        {ResponseCode::transaction_failed,
         proto::RPCRESPONSE_TRANSACTION_FAILED},
        {ResponseCode::unimplemented, proto::RPCRESPONSE_UNIMPLEMENTED},
        {ResponseCode::error, proto::RPCRESPONSE_ERROR},
    };

    return map;
}

auto translate(AccountEventType type) noexcept -> proto::AccountEventType
{
    try {

        return account_event_map().at(type);
    } catch (...) {

        return proto::ACCOUNTEVENT_ERROR;
    }
}
auto translate(AccountType type) noexcept -> proto::AccountType
{
    try {

        return account_map().at(type);
    } catch (...) {

        return proto::ACCOUNTTYPE_ERROR;
    }
}
auto translate(CommandType type) noexcept -> proto::RPCCommandType
{
    try {

        return command_map().at(type);
    } catch (...) {

        return proto::RPCCOMMAND_ERROR;
    }
}
auto translate(ContactEventType type) noexcept -> proto::ContactEventType
{
    try {

        return contact_event_map().at(type);
    } catch (...) {

        return proto::CONTACTEVENT_ERROR;
    }
}
auto translate(PaymentType type) noexcept -> proto::RPCPaymentType
{
    try {

        return payment_map().at(type);
    } catch (...) {

        return proto::RPCPAYMENTTYPE_ERROR;
    }
}
auto translate(PushType type) noexcept -> proto::RPCPushType
{
    try {

        return push_map().at(type);
    } catch (...) {

        return proto::RPCPUSH_ERROR;
    }
}
auto translate(ResponseCode type) noexcept -> proto::RPCResponseCode
{
    try {

        return response_code_map().at(type);
    } catch (...) {

        return proto::RPCRESPONSE_INVALID;
    }
}
auto translate(proto::AccountEventType type) noexcept -> AccountEventType
{
    static const auto map = reverse_arbitrary_map<
        AccountEventType,
        proto::AccountEventType,
        AccountEventReverseMap>(account_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return AccountEventType::error;
    }
}
auto translate(proto::AccountType type) noexcept -> AccountType
{
    static const auto map = reverse_arbitrary_map<
        AccountType,
        proto::AccountType,
        AccountReverseMap>(account_map());

    try {

        return map.at(type);
    } catch (...) {

        return AccountType::error;
    }
}
auto translate(proto::ContactEventType type) noexcept -> ContactEventType
{
    static const auto map = reverse_arbitrary_map<
        ContactEventType,
        proto::ContactEventType,
        ContactEventReverseMap>(contact_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return ContactEventType::error;
    }
}
auto translate(proto::RPCCommandType type) noexcept -> CommandType
{
    static const auto map = reverse_arbitrary_map<
        CommandType,
        proto::RPCCommandType,
        CommandReverseMap>(command_map());

    try {

        return map.at(type);
    } catch (...) {

        return CommandType::error;
    }
}
auto translate(proto::RPCPaymentType type) noexcept -> PaymentType
{
    static const auto map = reverse_arbitrary_map<
        PaymentType,
        proto::RPCPaymentType,
        PaymentReverseMap>(payment_map());

    try {

        return map.at(type);
    } catch (...) {

        return PaymentType::error;
    }
}
auto translate(proto::RPCPushType type) noexcept -> PushType
{
    static const auto map =
        reverse_arbitrary_map<PushType, proto::RPCPushType, PushReverseMap>(
            push_map());

    try {

        return map.at(type);
    } catch (...) {

        return PushType::error;
    }
}
auto translate(proto::RPCResponseCode type) noexcept -> ResponseCode
{
    static const auto map = reverse_arbitrary_map<
        ResponseCode,
        proto::RPCResponseCode,
        ResponseCodeReverseMap>(response_code_map());

    try {

        return map.at(type);
    } catch (...) {

        return ResponseCode::invalid;
    }
}
}  // namespace opentxs::rpc
