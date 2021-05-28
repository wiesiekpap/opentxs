// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "internal/rpc/RPC.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/intrusive/detail/iterator.hpp>
#include <boost/move/algo/detail/set_difference.hpp>
#include <boost/move/algo/move.hpp>
#include <functional>
#include <string>

#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/rpc/AccountEventType.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/ContactEventType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/PushType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/Types.hpp"
#include "util/Container.hpp"

namespace opentxs
{
auto print(rpc::AccountEventType value) noexcept -> std::string
{
    using Type = rpc::AccountEventType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::incoming_cheque, "incoming cheque"},
        {Type::outgoing_cheque, "outgoing cheque"},
        {Type::incoming_transfer, "incoming transfer"},
        {Type::outgoing_transfer, "outgoing transfer"},
        {Type::incoming_invoice, "incoming invoice"},
        {Type::outgoing_invoice, "outgoing invoice"},
        {Type::incoming_voucher, "incoming voucher"},
        {Type::outgoing_voucher, "outgoing voucher"},
        {Type::incoming_blockchain, "incoming blockchain"},
        {Type::outgoing_blockchain, "outgoing blockchain"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::AccountType value) noexcept -> std::string
{
    using Type = rpc::AccountType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::normal, "custodial"},
        {Type::issuer, "custodial issuer"},
        {Type::blockchain, "blockchain"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::CommandType value) noexcept -> std::string
{
    using Type = rpc::CommandType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::add_client_session, "add client session"},
        {Type::add_server_session, "add server session"},
        {Type::list_client_sessions, "list client sessions"},
        {Type::list_server_sessions, "list server sessions"},
        {Type::import_hd_seed, "import hd seed"},
        {Type::list_hd_seeds, "list hd seeds"},
        {Type::get_hd_seed, "get hd seed"},
        {Type::create_nym, "create nym"},
        {Type::list_nyms, "list nyms"},
        {Type::get_nym, "get nym"},
        {Type::add_claim, "add claim"},
        {Type::delete_claim, "delete claim"},
        {Type::import_server_contract, "import server contract"},
        {Type::list_server_contracts, "list server contracts"},
        {Type::register_nym, "register nym"},
        {Type::create_unit_definition, "create unit definition"},
        {Type::list_unit_definitions, "list unit definitions"},
        {Type::issue_unit_definition, "issue unit definition"},
        {Type::create_account, "create account"},
        {Type::list_accounts, "list accounts"},
        {Type::get_account_balance, "get account balance"},
        {Type::get_account_activity, "get account activity"},
        {Type::send_payment, "send payment"},
        {Type::move_funds, "move funds"},
        {Type::add_contact, "add contact"},
        {Type::list_contacts, "list contacts"},
        {Type::get_contact, "get contact"},
        {Type::add_contact_claim, "add contact claim"},
        {Type::delete_contact_claim, "delete contact claim"},
        {Type::verify_claim, "verify claim"},
        {Type::accept_verification, "accept verification"},
        {Type::send_contact_message, "send contact message"},
        {Type::get_contact_activity, "get contact activity"},
        {Type::get_server_contract, "get server contract"},
        {Type::get_pending_payments, "get pending payments"},
        {Type::accept_pending_payments, "accept pending payments"},
        {Type::get_compatible_accounts, "get compatible accounts"},
        {Type::create_compatible_account, "create compatible account"},
        {Type::get_workflow, "get workflow"},
        {Type::get_server_password, "get server password"},
        {Type::get_admin_nym, "get admin nym"},
        {Type::get_unit_definition, "get unit definition"},
        {Type::get_transaction_data, "get transaction data"},
        {Type::lookup_accountid, "lookup accountid"},
        {Type::rename_account, "rename account"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::ContactEventType value) noexcept -> std::string
{
    using Type = rpc::ContactEventType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::incoming_message, "incoming message"},
        {Type::outgoing_message, "outgoing message"},
        {Type::incoming_payment, "incoming payment"},
        {Type::outgoing_payment, "outgoing payment"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::PaymentType value) noexcept -> std::string
{
    using Type = rpc::PaymentType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::cheque, "cheque"},
        {Type::transfer, "transfer"},
        {Type::voucher, "voucher"},
        {Type::invoice, "invoice"},
        {Type::blinded, "blinded"},
        {Type::blockchain, "blockchain"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::PushType value) noexcept -> std::string
{
    using Type = rpc::PushType;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::account, "account"},
        {Type::contact, "contact"},
        {Type::task, "task"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(rpc::ResponseCode value) noexcept -> std::string
{
    using Type = rpc::ResponseCode;
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::invalid, "invalid"},
        {Type::success, "success"},
        {Type::bad_session, "bad_session"},
        {Type::none, "none"},
        {Type::queued, "queued"},
        {Type::unnecessary, "unnecessary"},
        {Type::retry, "retry"},
        {Type::no_path_to_recipient, "no path to recipient"},
        {Type::bad_server_argument, "bad server argument"},
        {Type::cheque_not_found, "cheque not found"},
        {Type::payment_not_found, "payment not found"},
        {Type::start_task_failed, "start task failed"},
        {Type::nym_not_found, "nym not found"},
        {Type::add_claim_failed, "add claim failed"},
        {Type::add_contact_failed, "add contact failed"},
        {Type::register_account_failed, "register account failed"},
        {Type::bad_server_response, "bad server response"},
        {Type::workflow_not_found, "workflow not found"},
        {Type::unit_definition_not_found, "unit definition not found"},
        {Type::session_not_found, "session not found"},
        {Type::create_nym_failed, "create nym failed"},
        {Type::create_unit_definition_failed, "create unit definition failed"},
        {Type::delete_claim_failed, "delete claim failed"},
        {Type::account_not_found, "account not found"},
        {Type::move_funds_failed, "move funds failed"},
        {Type::register_nym_failed, "register nym failed"},
        {Type::contact_not_found, "contact not found"},
        {Type::account_owner_not_found, "account owner not found"},
        {Type::send_payment_failed, "send payment failed"},
        {Type::transaction_failed, "transaction failed"},
        {Type::txid, "txid"},
        {Type::unimplemented, "unimplemented"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}
}  // namespace opentxs

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
        {ContactEventType::incoming_payment,
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
        {ResponseCode::txid, proto::RPCRESPONSE_TXID},
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
