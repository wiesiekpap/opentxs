// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "internal/core/Core.hpp"  // IWYU pragma: associated

#include <map>
#include <mutex>
#include <set>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/contact/Contact.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/protobuf/ContractEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::blockchain
{
auto AccountName([[maybe_unused]] const blockchain::Type chain) noexcept
    -> std::string
{
    return "This device";
}

auto Chain(const api::Core& api, const identifier::Nym& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::map<OTNymID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(IssuerID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto Chain(const api::Core& api, const identifier::Server& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::map<OTServerID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(NotaryID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto Chain(const api::Core& api, const identifier::UnitDefinition& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::map<OTUnitID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(UnitID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto IssuerID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const identifier::Nym&
{
    static auto mutex = std::mutex{};
    static auto map = std::map<blockchain::Type, OTNymID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().NymID());
    auto& output = it->second;

    try {
        const auto& hex =
            blockchain::params::Data::Chains().at(chain).genesis_hash_hex_;
        const auto genesis = api.Factory().Data(hex, StringStyle::Hex);
        output->CalculateDigest(genesis->Bytes());
    } catch (...) {
    }

    return output;
}

auto NotaryID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const identifier::Server&
{
    static auto mutex = std::mutex{};
    static auto map = std::map<blockchain::Type, OTServerID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().ServerID());
    auto& output = it->second;
    const auto preimage = std::string{"blockchain-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output->CalculateDigest(preimage);

    return output;
}

auto UnitID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const identifier::UnitDefinition&
{
    static auto mutex = std::mutex{};
    static auto map = std::map<blockchain::Type, OTUnitID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().UnitID());
    auto& output = it->second;

    try {
        const auto preimage =
            blockchain::params::Data::Chains().at(chain).display_ticker_;
        output->CalculateDigest(preimage);
    } catch (...) {
    }

    return output;
}
}  // namespace opentxs::blockchain

namespace opentxs::core
{

auto unittype_map() noexcept -> const UnitTypeMap&
{
    static const auto map = UnitTypeMap{
        {UnitType::Error, contact::ClaimType::Error},
        {UnitType::BTC, contact::ClaimType::BTC},
        {UnitType::ETH, contact::ClaimType::ETH},
        {UnitType::XRP, contact::ClaimType::XRP},
        {UnitType::LTC, contact::ClaimType::LTC},
        {UnitType::DAO, contact::ClaimType::DAO},
        {UnitType::XEM, contact::ClaimType::XEM},
        {UnitType::DASH, contact::ClaimType::DASH},
        {UnitType::MAID, contact::ClaimType::MAID},
        {UnitType::LSK, contact::ClaimType::LSK},
        {UnitType::DOGE, contact::ClaimType::DOGE},
        {UnitType::DGD, contact::ClaimType::DGD},
        {UnitType::XMR, contact::ClaimType::XMR},
        {UnitType::WAVES, contact::ClaimType::WAVES},
        {UnitType::NXT, contact::ClaimType::NXT},
        {UnitType::SC, contact::ClaimType::SC},
        {UnitType::STEEM, contact::ClaimType::STEEM},
        {UnitType::AMP, contact::ClaimType::AMP},
        {UnitType::XLM, contact::ClaimType::XLM},
        {UnitType::FCT, contact::ClaimType::FCT},
        {UnitType::BTS, contact::ClaimType::BTS},
        {UnitType::USD, contact::ClaimType::USD},
        {UnitType::EUR, contact::ClaimType::EUR},
        {UnitType::GBP, contact::ClaimType::GBP},
        {UnitType::INR, contact::ClaimType::INR},
        {UnitType::AUD, contact::ClaimType::AUD},
        {UnitType::CAD, contact::ClaimType::CAD},
        {UnitType::SGD, contact::ClaimType::SGD},
        {UnitType::CHF, contact::ClaimType::CHF},
        {UnitType::MYR, contact::ClaimType::MYR},
        {UnitType::JPY, contact::ClaimType::JPY},
        {UnitType::CNY, contact::ClaimType::CNY},
        {UnitType::NZD, contact::ClaimType::NZD},
        {UnitType::THB, contact::ClaimType::THB},
        {UnitType::HUF, contact::ClaimType::HUF},
        {UnitType::AED, contact::ClaimType::AED},
        {UnitType::HKD, contact::ClaimType::HKD},
        {UnitType::MXN, contact::ClaimType::MXN},
        {UnitType::ZAR, contact::ClaimType::ZAR},
        {UnitType::PHP, contact::ClaimType::PHP},
        {UnitType::SEC, contact::ClaimType::SEC},
        {UnitType::TNBTC, contact::ClaimType::TNBTC},
        {UnitType::TNXRP, contact::ClaimType::TNXRP},
        {UnitType::TNLTX, contact::ClaimType::TNLTX},
        {UnitType::TNXEM, contact::ClaimType::TNXEM},
        {UnitType::TNDASH, contact::ClaimType::TNDASH},
        {UnitType::TNMAID, contact::ClaimType::TNMAID},
        {UnitType::TNLSK, contact::ClaimType::TNLSK},
        {UnitType::TNDOGE, contact::ClaimType::TNDOGE},
        {UnitType::TNXMR, contact::ClaimType::TNXMR},
        {UnitType::TNWAVES, contact::ClaimType::TNWAVES},
        {UnitType::TNNXT, contact::ClaimType::TNNXT},
        {UnitType::TNSC, contact::ClaimType::TNSC},
        {UnitType::TNSTEEM, contact::ClaimType::TNSTEEM},
        {UnitType::BCH, contact::ClaimType::BCH},
        {UnitType::TNBCH, contact::ClaimType::TNBCH},
        {UnitType::PKT, contact::ClaimType::PKT},
        {UnitType::TNPKT, contact::ClaimType::TNPKT},
        {UnitType::Ethereum_Olympic, contact::ClaimType::Ethereum_Olympic},
        {UnitType::Ethereum_Classic, contact::ClaimType::Ethereum_Classic},
        {UnitType::Ethereum_Expanse, contact::ClaimType::Ethereum_Expanse},
        {UnitType::Ethereum_Morden, contact::ClaimType::Ethereum_Morden},
        {UnitType::Ethereum_Ropsten, contact::ClaimType::Ethereum_Ropsten},
        {UnitType::Ethereum_Rinkeby, contact::ClaimType::Ethereum_Rinkeby},
        {UnitType::Ethereum_Kovan, contact::ClaimType::Ethereum_Kovan},
        {UnitType::Ethereum_Sokol, contact::ClaimType::Ethereum_Sokol},
        {UnitType::Ethereum_POA, contact::ClaimType::Ethereum_POA},
        {UnitType::Regtest, contact::ClaimType::Regtest},
        {UnitType::Unknown, contact::ClaimType::Unknown},
    };

    return map;
}

auto translate(const UnitType in) noexcept -> contact::ClaimType
{
    try {
        return unittype_map().at(in);
    } catch (...) {
        return contact::ClaimType::Error;
    }
}
auto translate(const contact::ClaimType in) noexcept -> UnitType
{
    static const auto map =
        reverse_arbitrary_map<UnitType, contact::ClaimType, UnitTypeReverseMap>(
            unittype_map());

    try {
        return map.at(in);
    } catch (...) {
        return UnitType::Error;
    }
}
}  // namespace opentxs::core

namespace opentxs::core::internal
{
auto addresstype_map() noexcept -> const AddressTypeMap&
{
    static const auto map = AddressTypeMap{
        {AddressType::Error, proto::ADDRESSTYPE_ERROR},
        {AddressType::IPV4, proto::ADDRESSTYPE_IPV4},
        {AddressType::IPV6, proto::ADDRESSTYPE_IPV6},
        {AddressType::Onion, proto::ADDRESSTYPE_ONION},
        {AddressType::EEP, proto::ADDRESSTYPE_EEP},
        {AddressType::Inproc, proto::ADDRESSTYPE_INPROC},
    };

    return map;
}

auto translate(AddressType in) noexcept -> proto::AddressType
{
    try {
        return addresstype_map().at(in);
    } catch (...) {
        return proto::ADDRESSTYPE_ERROR;
    }
}

auto translate(proto::AddressType in) noexcept -> AddressType
{
    static const auto map = reverse_arbitrary_map<
        AddressType,
        proto::AddressType,
        AddressTypeReverseMap>(addresstype_map());

    try {
        return map.at(in);
    } catch (...) {
        return AddressType::Error;
    }
}

auto translate(const UnitType in) noexcept -> proto::ContactItemType
{
    return contact::internal::translate(core::translate(in));
}

auto translate(const proto::ContactItemType in) noexcept -> UnitType
{
    return core::translate(contact::internal::translate(in));
}

}  // namespace opentxs::core::internal
