// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "server/MainFile.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "core/OTStorage.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/identity/Nym.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"

#define OT_METHOD "opentxs::Mainfile::"

namespace opentxs::server
{
MainFile::MainFile(Server& server, const PasswordPrompt& reason)
    : server_(server)
    , version_()
{
}

auto MainFile::SaveMainFileToString(String& strMainFile) -> bool
{
    Tag tag("notaryServer");

    tag.add_attribute("version", "3.0");
    tag.add_attribute("notaryID", server_.GetServerID().str());
    tag.add_attribute("serverNymID", server_.GetServerNym().ID().str());
    tag.add_attribute(
        "transactionNum",
        std::to_string(server_.GetTransactor().transactionNumber()));

    // Save the basket account information

    for (auto& it : server_.GetTransactor().idToBasketMap_) {
        auto strBasketID = String::Factory(it.first.c_str());
        auto strBasketAcctID = String::Factory(it.second.c_str());

        const auto BASKET_ACCOUNT_ID =
            server_.API().Factory().Identifier(strBasketAcctID);
        auto BASKET_CONTRACT_ID = server_.API().Factory().UnitID();

        bool bContractID =
            server_.GetTransactor().lookupBasketContractIDByAccountID(
                BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID);

        if (!bContractID) {
            LogOutput(OT_METHOD)(__func__)(
                ": Error: Missing Contract ID for basket ID ")(
                strBasketID->Get())(".")
                .Flush();
            break;
        }

        auto strBasketContractID = String::Factory((BASKET_CONTRACT_ID));

        TagPtr pTag(new Tag("basketInfo"));

        pTag->add_attribute("basketID", strBasketID->Get());
        pTag->add_attribute("basketAcctID", strBasketAcctID->Get());
        pTag->add_attribute("basketContractID", strBasketContractID->Get());

        tag.add_tag(pTag);
    }

    server_.GetTransactor().voucherAccounts_.Serialize(tag);

    std::string str_result;
    tag.output(str_result);

    strMainFile.Concatenate("%s", str_result.c_str());

    return true;
}

// Setup the default location for the Sever Main File...
// maybe this should be set differently...
// should be set in the servers configuration.
//
auto MainFile::SaveMainFile() -> bool
{
    // Get the loaded (or new) version of the Server's Main File.
    //
    auto strMainFile = String::Factory();

    if (!SaveMainFileToString(strMainFile)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Error saving to string. (Giving up on save attempt).")
            .Flush();
        return false;
    }
    // Try to save the notary server's main datafile to local storage...
    //
    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(strMainFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, "NOTARY"))  // todo
                                                          // hardcoding.
    {
        LogOutput(OT_METHOD)(__func__)(
            ": Error saving notary (Failed writing armored string).")
            .Flush();
        return false;
    }
    // Save the Main File to the Harddrive... (or DB, if other storage module is
    // being used).
    //
    const bool bSaved = OTDB::StorePlainString(
        server_.API(),
        strFinal->Get(),
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        "");

    if (!bSaved) {
        LogOutput(OT_METHOD)(__func__)(": Error saving main file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
    }
    return bSaved;
}

auto MainFile::CreateMainFile(
    const std::string& strContract,
    const std::string& strNotaryID,
    const std::string& strNymID) -> bool
{
    if (!OTDB::StorePlainString(
            server_.API(),
            strContract,
            server_.API().DataFolder(),
            server_.API().Internal().Legacy().Contract(),
            strNotaryID,
            "",
            "")) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed trying to store the server contract.")
            .Flush();
        return false;
    }

    const char* szBlankFile =  // todo hardcoding.
        "<notaryServer version=\"2.0\"\n"
        " notaryID=\"%s\"\n"
        " serverNymID=\"%s\"\n"
        " transactionNum=\"%ld\" >\n"
        "\n"
        "<accountList type=\"voucher\" count=\"0\" >\n"
        "\n"
        "</accountList>\n"
        "\n"
        "</notaryServer>\n\n";

    std::int64_t lTransNum = 5;  // a starting point, for the new server.

    auto strNotaryFile = String::Factory();
    strNotaryFile->Format(
        szBlankFile, strNotaryID.c_str(), strNymID.c_str(), lTransNum);

    std::string str_Notary(strNotaryFile->Get());

    if (!OTDB::StorePlainString(
            server_.API(),
            str_Notary,
            server_.API().DataFolder(),
            ".",
            "notaryServer.xml",
            "",
            ""))  // todo hardcoding.
    {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed trying to store the new notaryServer.xml file.")
            .Flush();
        return false;
    }

    // At this point, the contract is saved, the cert is saved, and the
    // notaryServer.xml file
    // is saved. All we have left is the Nymfile, which we'll create.

    auto loaded =
        server_.LoadServerNym(server_.API().Factory().NymID(strNymID));
    if (false == loaded) {
        LogNormal(OT_METHOD)(__func__)(": Error loading server nym.").Flush();
    } else {
        LogVerbose(OT_METHOD)(__func__)(
            ": OKAY, we have apparently created the new "
            "server. "
            "Let's try to load up your new server contract...")
            .Flush();

        return true;
    }
    return false;
}

auto MainFile::LoadMainFile(bool bReadOnly) -> bool
{
    if (!OTDB::Exists(
            server_.API(),
            server_.API().DataFolder(),
            ".",
            server_.WalletFilename().Get(),
            "",
            "")) {
        LogOutput(OT_METHOD)(__func__)(": Error finding file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
        return false;
    }
    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        server_.API(),
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (!strFileContents->Exists()) {
        LogOutput(OT_METHOD)(__func__)(": Unable to read main file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
        return false;
    }

    bool bNeedToSaveAgain = false;

    bool bFailure = false;

    {
        auto xmlFileContents = StringXML::Factory(strFileContents);

        if (false == xmlFileContents->DecodeIfArmored()) {
            LogOutput(OT_METHOD)(__func__)(
                ": Notary server file apparently was encoded and "
                "then failed decoding. Filename: ")(
                server_.WalletFilename().Get())(
                ". "
                "Contents:"
                " ")(strFileContents->Get())(".")
                .Flush();
            return false;
        }

        irr::io::IrrXMLReader* xml =
            irr::io::createIrrXMLReader(xmlFileContents.get());
        std::unique_ptr<irr::io::IrrXMLReader> theXMLGuardian(xml);

        while (xml && xml->read()) {
            // strings for storing the data that we want to read out of the file
            auto AssetName = String::Factory();
            auto InstrumentDefinitionID = String::Factory();
            const auto strNodeName = String::Factory(xml->getNodeName());

            switch (xml->getNodeType()) {
                case irr::io::EXN_TEXT:
                    break;
                case irr::io::EXN_ELEMENT: {
                    if (strNodeName->Compare("notaryServer")) {
                        version_ = xml->getAttributeValue("version");
                        server_.SetNotaryID(
                            server_.API().Factory().ServerID(String::Factory(
                                xml->getAttributeValue("notaryID"))));
                        server_.SetServerNymID(
                            xml->getAttributeValue("serverNymID"));

                        auto strTransactionNumber =
                            String::Factory();  // The server issues
                                                // transaction numbers and
                                                // stores the counter here
                                                // for the latest one.
                        strTransactionNumber = String::Factory(
                            xml->getAttributeValue("transactionNum"));
                        server_.GetTransactor().transactionNumber(
                            strTransactionNumber->ToLong());
                        LogNormal("Loading Open Transactions server").Flush();
                        LogNormal("* File version: ")(version_).Flush();
                        LogNormal("* Last Issued Transaction Number: ")(
                            server_.GetTransactor().transactionNumber())
                            .Flush();
                        LogNormal("* Notary ID: ")(server_.GetServerID().str())
                            .Flush();
                        LogNormal("* Server Nym ID: ")(server_.ServerNymID())
                            .Flush();
                        // the voucher reserve account IDs.
                    } else if (strNodeName->Compare("accountList")) {
                        const auto strAcctType =
                            String::Factory(xml->getAttributeValue("type"));
                        const auto strAcctCount =
                            String::Factory(xml->getAttributeValue("count"));

                        if ((-1) == server_.GetTransactor()
                                        .voucherAccounts_.ReadFromXMLNode(
                                            xml, strAcctType, strAcctCount))
                            LogOutput(OT_METHOD)(__func__)(
                                ": Error loading voucher accountList.")
                                .Flush();
                    } else if (strNodeName->Compare("basketInfo")) {
                        auto strBasketID =
                            String::Factory(xml->getAttributeValue("basketID"));
                        auto strBasketAcctID = String::Factory(
                            xml->getAttributeValue("basketAcctID"));
                        auto strBasketContractID = String::Factory(
                            xml->getAttributeValue("basketContractID"));
                        const auto BASKET_ID =
                            server_.API().Factory().Identifier(strBasketID);
                        const auto BASKET_ACCT_ID =
                            server_.API().Factory().Identifier(strBasketAcctID);
                        const auto BASKET_CONTRACT_ID =
                            server_.API().Factory().UnitID(strBasketContractID);

                        if (server_.GetTransactor().addBasketAccountID(
                                BASKET_ID,
                                BASKET_ACCT_ID,
                                BASKET_CONTRACT_ID)) {
                            LogNormal(OT_METHOD)(__func__)(
                                ": Loading basket currency info... "
                                "Basket ID: ")(strBasketID)(" Basket Acct "
                                                            "ID:"
                                                            " ")(
                                strBasketAcctID)(" Basket Contract ID: ")(
                                strBasketContractID)(".")
                                .Flush();
                        } else {
                            LogOutput(OT_METHOD)(__func__)(
                                ": Error adding basket currency info."
                                " Basket ID: ")(strBasketID->Get())(
                                ". Basket Acct "
                                "ID:"
                                " ")(strBasketAcctID->Get())(".")
                                .Flush();
                        }
                    } else {
                        // unknown element type
                        LogOutput(OT_METHOD)(__func__)(
                            ": Unknown element type: ")(xml->getNodeName())(".")
                            .Flush();
                    }
                } break;
                default: {
                }
            }
        }
    }

    if (server_.ServerNymID().empty()) {
        LogOutput(OT_METHOD)(__func__)(": Failed to determine server nym id.")
            .Flush();
        bFailure = true;
    }

    if (false == bFailure) {
        const auto loaded = server_.LoadServerNym(
            server_.API().Factory().NymID(server_.ServerNymID()));

        if (false == loaded) {
            LogOutput(OT_METHOD)(__func__)(": Failed to load server nym.")
                .Flush();
            bFailure = true;
        }
    }

    if (false == bFailure) {
        const auto loaded = LoadServerUserAndContract();

        if (false == loaded) {
            LogOutput(OT_METHOD)(__func__)(": Failed to load nym.").Flush();
            bFailure = true;
        }
    }

    if (false == bReadOnly) {
        if (bNeedToSaveAgain) { SaveMainFile(); }
    }

    return !bFailure;
}

auto MainFile::LoadServerUserAndContract() -> bool
{
    bool bSuccess = false;
    auto& serverNym = server_.m_nymServer;

    OT_ASSERT(!version_.empty());
    OT_ASSERT(!server_.GetServerID().str().empty());
    OT_ASSERT(!server_.ServerNymID().empty());

    serverNym = server_.API().Wallet().Nym(
        server_.API().Factory().NymID(server_.ServerNymID()));

    if (serverNym->HasCapability(NymCapability::SIGN_MESSAGE)) {
        LogTrace(OT_METHOD)(__func__)(": Server nym is viable.").Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Server nym lacks private keys.")
            .Flush();

        return false;
    }

    // Load Cron (now that we have the server Nym.
    // (I WAS loading this erroneously in Server.Init(), before
    // the Nym had actually been loaded from disk. That didn't work.)
    //
    const auto& NOTARY_ID = server_.GetServerID();

    // Make sure the Cron object has a pointer to the server's Nym.
    // (For signing stuff...)
    //
    server_.Cron().SetNotaryID(NOTARY_ID);
    server_.Cron().SetServerNym(serverNym);

    if (!server_.Cron().LoadCron()) {
        LogDetail(OT_METHOD)(__func__)(
            ": Failed loading Cron file. (Did you just create this "
            "server?).")
            .Flush();
    }

    LogDetail(OT_METHOD)(__func__)(": Loading the server contract...").Flush();

    try {
        server_.API().Wallet().Server(NOTARY_ID);
        bSuccess = true;
        LogDetail(OT_METHOD)(__func__)(": ** Main Server Contract Verified **")
            .Flush();
    } catch (...) {
        LogNormal(OT_METHOD)(__func__)(
            ": Failed reading Main Server Contract: ")
            .Flush();
    }

    return bSuccess;
}
}  // namespace opentxs::server
