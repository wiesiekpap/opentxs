// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/core/script/OTStash.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/script/OTStashItem.hpp"
#include "opentxs/core/util/Tag.hpp"

#define OT_METHOD "opentxs::OTStash"

namespace opentxs
{
OTStash::OTStash()
    : m_str_stash_name()
    , m_mapStashItems()
{
}

OTStash::OTStash(const std::string& str_stash_name)
    : m_str_stash_name(str_stash_name)
    , m_mapStashItems()
{
}

OTStash::OTStash(const String& strInstrumentDefinitionID, std::int64_t lAmount)
    : m_str_stash_name()
    , m_mapStashItems()
{
    auto* pItem = new OTStashItem(strInstrumentDefinitionID, lAmount);
    OT_ASSERT(nullptr != pItem);

    m_mapStashItems.insert(std::pair<std::string, OTStashItem*>(
        strInstrumentDefinitionID.Get(), pItem));
}

OTStash::OTStash(
    const Identifier& theInstrumentDefinitionID,
    std::int64_t lAmount)
    : m_str_stash_name()
    , m_mapStashItems()
{
    auto* pItem = new OTStashItem(theInstrumentDefinitionID, lAmount);
    OT_ASSERT(nullptr != pItem);

    auto strInstrumentDefinitionID = String::Factory(theInstrumentDefinitionID);

    m_mapStashItems.insert(std::pair<std::string, OTStashItem*>(
        strInstrumentDefinitionID->Get(), pItem));
}

void OTStash::Serialize(Tag& parent) const
{
    const auto sizeMapStashItems = m_mapStashItems.size();

    TagPtr pTag(new Tag("stash"));

    pTag->add_attribute("name", m_str_stash_name);
    pTag->add_attribute("count", std::to_string(sizeMapStashItems));

    for (auto& it : m_mapStashItems) {
        const std::string str_instrument_definition_id = it.first;
        OTStashItem* pStashItem = it.second;
        OT_ASSERT(
            (str_instrument_definition_id.size() > 0) &&
            (nullptr != pStashItem));

        TagPtr pTagItem(new Tag("stashItem"));

        pTagItem->add_attribute(
            "instrumentDefinitionID",
            pStashItem->GetInstrumentDefinitionID().Get());
        pTagItem->add_attribute(
            "balance", std::to_string(pStashItem->GetAmount()));

        pTag->add_tag(pTagItem);
    }

    parent.add_tag(pTag);
}

auto OTStash::ReadFromXMLNode(
    irr::io::IrrXMLReader*& xml,
    const String& strStashName,
    const String& strItemCount) -> std::int32_t
{
    if (!strStashName.Exists()) {
        LogOutput(OT_METHOD)(__func__)(": Failed: Empty stash 'name' "
                                       "attribute.")
            .Flush();
        return (-1);
    }

    m_str_stash_name = strStashName.Get();

    //
    // Load up the stash items.
    //
    std::int32_t nCount = strItemCount.Exists() ? atoi(strItemCount.Get()) : 0;
    if (nCount > 0) {
        while (nCount-- > 0) {
            //            xml->read();
            if (!Contract::SkipToElement(xml)) {
                LogNormal(OT_METHOD)(__func__)(": Failure: Unable to find "
                                               "expected element.")
                    .Flush();
                return (-1);
            }

            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                (!strcmp("stashItem", xml->getNodeName()))) {
                auto strInstrumentDefinitionID =
                    String::Factory(xml->getAttributeValue(
                        "instrumentDefinitionID"));  // Instrument Definition ID
                                                     // of this account.
                auto strAmount = String::Factory(xml->getAttributeValue(
                    "balance"));  // Account ID for this account.

                if (!strInstrumentDefinitionID->Exists() ||
                    !strAmount->Exists()) {
                    LogOutput(OT_METHOD)(__func__)(
                        ": Error loading "
                        "stashItem: Either the instrumentDefinitionID (")(
                        strInstrumentDefinitionID)("), or the balance (")(
                        strAmount)(") was EMPTY.")
                        .Flush();
                    return (-1);
                }

                if (!CreditStash(
                        strInstrumentDefinitionID->Get(),
                        strAmount->ToLong()))  // <===============
                {
                    LogOutput(OT_METHOD)(__func__)(": Failed crediting "
                                                   "stashItem for stash ")(
                        strStashName)(". instrumentDefinitionID (")(
                        strInstrumentDefinitionID)("), balance (")(
                        strAmount)(").")
                        .Flush();
                    return (-1);
                }

                // (Success)
            } else {
                LogOutput(OT_METHOD)(__func__)(": Expected stashItem "
                                               "element.")
                    .Flush();
                return (-1);  // error condition
            }
        }  // while
    }

    if (!Contract::SkipAfterLoadingField(xml))  // </stash>
    {
        LogNormal(OT_METHOD)(__func__)(": Bad data? Expected "
                                       "EXN_ELEMENT_END here, but "
                                       "didn't get it. Returning -1.")
            .Flush();
        return (-1);
    }

    return 1;
}

// Creates it if it's not already there.
// (*this owns it and will clean it up when destroyed.)
//
auto OTStash::GetStash(const std::string& str_instrument_definition_id)
    -> OTStashItem*
{
    auto it = m_mapStashItems.find(str_instrument_definition_id);

    if (m_mapStashItems.end() == it)  // It's not already there for this
                                      // instrument definition.
    {
        const auto strInstrumentDefinitionID =
            String::Factory(str_instrument_definition_id.c_str());
        auto* pStashItem = new OTStashItem(strInstrumentDefinitionID);
        OT_ASSERT(nullptr != pStashItem);

        m_mapStashItems.insert(std::pair<std::string, OTStashItem*>(
            strInstrumentDefinitionID->Get(), pStashItem));
        return pStashItem;
    }

    OTStashItem* pStashItem = it->second;
    OT_ASSERT(nullptr != pStashItem);

    return pStashItem;
}

auto OTStash::GetAmount(const std::string& str_instrument_definition_id)
    -> std::int64_t
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT()
                                                 // if failure.)

    return pStashItem->GetAmount();
}

auto OTStash::CreditStash(
    const std::string& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT()
                                                 // if failure.)

    return pStashItem->CreditStash(lAmount);
}

auto OTStash::DebitStash(
    const std::string& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT()
                                                 // if failure.)

    return pStashItem->DebitStash(lAmount);
}

OTStash::~OTStash()
{
    while (!m_mapStashItems.empty()) {
        OTStashItem* pTemp = m_mapStashItems.begin()->second;
        OT_ASSERT(nullptr != pTemp);
        delete pTemp;
        pTemp = nullptr;
        m_mapStashItems.erase(m_mapStashItems.begin());
    }
}
}  // namespace opentxs
