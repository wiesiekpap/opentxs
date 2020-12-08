// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/blockchain/AddressStyle.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace ot = opentxs;

using Style = ot::api::client::blockchain::AddressStyle;
using Chain = ot::blockchain::Type;
using TestData = std::map<std::string, std::pair<Style, std::set<Chain>>>;

// https://en.bitcoin.it/wiki/Technical_background_of_version_1_Bitcoin_addresses
// https://bitcoin.stackexchange.com/questions/62781/litecoin-constants-and-prefixes
const auto vector_ = TestData{
    {"17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem",
     {Style::P2PKH, {Chain::Bitcoin, Chain::BitcoinCash}}},
    {"3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX",
     {Style::P2SH, {Chain::Bitcoin, Chain::BitcoinCash, Chain::Litecoin}}},
    {"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn",
     {Style::P2PKH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet}}},
    {"2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc",
     {Style::P2SH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet}}},
    {"LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1", {Style::P2PKH, {Chain::Litecoin}}},
    {"3MSvaVbVFFLML86rt5eqgA9SvW23upaXdY",
     {Style::P2SH, {Chain::Bitcoin, Chain::BitcoinCash, Chain::Litecoin}}},
    {"MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh", {Style::P2SH, {Chain::Litecoin}}},
    {"2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16",
     {Style::P2SH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet}}},
    {"QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK",
     {Style::P2SH, {Chain::Litecoin_testnet4}}},
    {"pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn", {Style::P2PKH, {Chain::PKT}}},
};

namespace
{
class Test_Address : public ::testing::Test
{
public:
    const ot::api::client::Manager& api_;

    Test_Address()
        : api_(ot::Context().StartClient({}, 0))
    {
    }
};

TEST_F(Test_Address, decode)
{
    for (const auto& [address, data] : vector_) {
        const auto& [expectedStyle, expectedChains] = data;
        const auto [bytes, style, chains] =
            api_.Blockchain().DecodeAddress(address);

        EXPECT_EQ(style, expectedStyle);
        EXPECT_EQ(chains.size(), expectedChains.size());

        for (const auto& chain : expectedChains) {
            EXPECT_EQ(chains.count(chain), 1);
        }
    }
}
}  // namespace
