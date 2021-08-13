// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "Helpers.hpp"
#include "opentxs/api/Options.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

constexpr auto blockchain_1_{opentxs::blockchain::Type::Bitcoin};
constexpr auto blockchain_2_{opentxs::blockchain::Type::Litecoin};
constexpr auto blockchain_storage_level_1_{1};
constexpr auto blockchain_storage_level_2_{3};
constexpr auto blockchain_sync_enabled_1_{true};
constexpr auto blockchain_sync_enabled_2_{false};
constexpr auto blockchain_wallet_enabled_1_{false};
constexpr auto blockchain_wallet_enabled_2_{true};
constexpr auto home_1_{"/home/user1/.opentxs"};
constexpr auto home_2_{"/home/user2/.opentxs"};
constexpr auto log_endpoint_1_{"inproc://send_logs_here_plz"};
constexpr auto log_endpoint_2_{"inproc://actually_send_them_here"};
constexpr auto log_leve_1_{2};
constexpr auto log_leve_2_{4};
constexpr auto notary_bind_inproc_1_{true};
constexpr auto notary_bind_inproc_2_{false};
constexpr auto notary_bind_ip_1_{"127.0.0.1"};
constexpr auto notary_bind_ip_2_{"127.0.0.2"};
constexpr auto notary_bind_port_1_{1111};
constexpr auto notary_bind_port_2_{1112};
constexpr auto notary_public_port_1_{2222};
constexpr auto notary_public_port_2_{2223};
constexpr auto notary_name_1_{"Notary 1"};
constexpr auto notary_name_2_{"Notary 2"};
constexpr auto notary_name_3_{"Notary 3"};
constexpr auto notary_public_eep_1_{"I forgot the format of an eepsite"};
constexpr auto notary_public_eep_2_{"something.i2p probably"};
constexpr auto notary_public_ipv4_1_{"8.8.8.8"};
constexpr auto notary_public_ipv4_2_{"9.9.9.9"};
constexpr auto notary_public_ipv6_1_{"2001:db8:3333:4444:5555:6666:7777:8888"};
constexpr auto notary_public_ipv6_2_{"2001:db8:3333:4444:CCCC:DDDD:EEEE:FFFF"};
constexpr auto notary_public_opion_1_{"something.onion"};
constexpr auto notary_public_opion_2_{"something else.onion"};
constexpr auto notary_terms_1_{"Don't be evil"};
constexpr auto notary_terms_2_{"lol jk"};
constexpr auto storage_plugin_1_{"sqlite"};
constexpr auto storage_plugin_2_{"fs"};
constexpr auto sync_server_1_{"tcp://some.ip:1234"};
constexpr auto sync_server_2_{"tcp://another.ip:1234"};

namespace ottest
{
TEST(Options, default_values)
{
    const auto test = opentxs::Options{};
    const auto expected = OptionsData{
        {},
        0,
        false,
        {},
        true,
        "",
        "",
        0,
        false,
        "",
        0,
        "",
        {},
        {},
        {},
        {},
        0,
        "",
        ""};

    EXPECT_TRUE(check_options(test, expected));
}

TEST(Options, setters)
{
    const auto test1 =
        opentxs::Options{}
            .AddBlockchainSyncServer(sync_server_1_)
            .AddNotaryPublicEEP(notary_public_eep_1_)
            .AddNotaryPublicIPv4(notary_public_ipv4_1_)
            .AddNotaryPublicIPv6(notary_public_ipv6_1_)
            .AddNotaryPublicOnion(notary_public_opion_1_)
            .DisableBlockchain(blockchain_1_)
            .SetBlockchainStorageLevel(blockchain_storage_level_1_)
            .SetBlockchainSyncEnabled(blockchain_sync_enabled_1_)
            .SetBlockchainWalletEnabled(blockchain_wallet_enabled_1_)
            .SetHome(home_1_)
            .SetLogEndpoint(log_endpoint_1_)
            .SetLogLevel(log_leve_1_)
            .SetNotaryBindIP(notary_bind_ip_1_)
            .SetNotaryBindPort(notary_bind_port_1_)
            .SetNotaryInproc(notary_bind_inproc_1_)
            .SetNotaryName(notary_name_1_)
            .SetNotaryPublicPort(notary_public_port_1_)
            .SetNotaryTerms(notary_terms_1_)
            .SetStoragePlugin(storage_plugin_1_);
    const auto test2{test1};
    const auto test3 =
        opentxs::Options{test2}
            .AddBlockchainSyncServer(sync_server_2_)
            .AddNotaryPublicEEP(notary_public_eep_2_)
            .AddNotaryPublicIPv4(notary_public_ipv4_2_)
            .AddNotaryPublicIPv6(notary_public_ipv6_2_)
            .AddNotaryPublicOnion(notary_public_opion_2_)
            .DisableBlockchain(blockchain_2_)
            .SetBlockchainStorageLevel(blockchain_storage_level_2_)
            .SetBlockchainSyncEnabled(blockchain_sync_enabled_2_)
            .SetBlockchainWalletEnabled(blockchain_wallet_enabled_2_)
            .SetHome(home_2_)
            .SetLogEndpoint(log_endpoint_2_)
            .SetLogLevel(log_leve_2_)
            .SetNotaryBindIP(notary_bind_ip_2_)
            .SetNotaryBindPort(notary_bind_port_2_)
            .SetNotaryInproc(notary_bind_inproc_2_)
            .SetNotaryName(notary_name_2_)
            .SetNotaryPublicPort(notary_public_port_2_)
            .SetNotaryTerms(notary_terms_2_)
            .SetStoragePlugin(storage_plugin_2_);

    const auto expected1 = OptionsData{
        {blockchain_1_},
        blockchain_storage_level_1_,
        blockchain_sync_enabled_1_,
        {sync_server_1_},
        blockchain_wallet_enabled_1_,
        home_1_,
        log_endpoint_1_,
        log_leve_1_,
        notary_bind_inproc_1_,
        notary_bind_ip_1_,
        notary_bind_port_1_,
        notary_name_1_,
        {notary_public_eep_1_},
        {notary_public_ipv4_1_},
        {notary_public_ipv6_1_},
        {notary_public_opion_1_},
        notary_public_port_1_,
        notary_terms_1_,
        storage_plugin_1_};
    const auto expected2 = OptionsData{
        {blockchain_1_, blockchain_2_},
        blockchain_storage_level_2_,
        blockchain_sync_enabled_2_,
        {sync_server_1_, sync_server_2_},
        blockchain_wallet_enabled_2_,
        home_2_,
        log_endpoint_2_,
        log_leve_2_,
        notary_bind_inproc_2_,
        notary_bind_ip_2_,
        notary_bind_port_2_,
        notary_name_2_,
        {notary_public_eep_1_, notary_public_eep_2_},
        {notary_public_ipv4_1_, notary_public_ipv4_2_},
        {notary_public_ipv6_1_, notary_public_ipv6_2_},
        {notary_public_opion_1_, notary_public_opion_2_},
        notary_public_port_2_,
        notary_terms_2_,
        storage_plugin_2_};

    EXPECT_TRUE(check_options(test1, expected1));
    EXPECT_TRUE(check_options(test2, expected1));
    EXPECT_TRUE(check_options(test3, expected2));
}

TEST(Options, merge)
{
    const auto blank = opentxs::Options{};
    const auto test1 =
        opentxs::Options{}
            .AddNotaryPublicEEP(notary_public_eep_1_)
            .AddNotaryPublicIPv4(notary_public_ipv4_1_)
            .AddNotaryPublicIPv6(notary_public_ipv6_1_)
            .AddNotaryPublicOnion(notary_public_opion_1_)
            .SetBlockchainStorageLevel(blockchain_storage_level_1_)
            .SetBlockchainSyncEnabled(blockchain_sync_enabled_1_)
            .SetBlockchainWalletEnabled(blockchain_wallet_enabled_1_)
            .SetHome(home_1_)
            .SetLogEndpoint(log_endpoint_1_)
            .SetLogLevel(log_leve_1_)
            .SetNotaryBindIP(notary_bind_ip_1_)
            .SetNotaryBindPort(notary_bind_port_1_)
            .SetNotaryInproc(notary_bind_inproc_1_)
            .SetNotaryName(notary_name_1_)
            .SetNotaryPublicPort(notary_public_port_1_)
            .SetNotaryTerms(notary_terms_1_)
            .SetStoragePlugin(storage_plugin_1_);
    const auto test2 =
        opentxs::Options{}
            .AddBlockchainSyncServer(sync_server_2_)
            .AddNotaryPublicEEP(notary_public_eep_2_)
            .AddNotaryPublicIPv4(notary_public_ipv4_2_)
            .AddNotaryPublicIPv6(notary_public_ipv6_2_)
            .AddNotaryPublicOnion(notary_public_opion_2_)
            .DisableBlockchain(blockchain_2_)
            .SetBlockchainStorageLevel(blockchain_storage_level_2_)
            .SetBlockchainSyncEnabled(blockchain_sync_enabled_2_)
            .SetBlockchainWalletEnabled(blockchain_wallet_enabled_2_)
            .SetHome(home_2_)
            .SetLogEndpoint(log_endpoint_2_)
            .SetLogLevel(log_leve_2_)
            .SetNotaryBindIP(notary_bind_ip_2_)
            .SetNotaryBindPort(notary_bind_port_2_)
            .SetNotaryInproc(notary_bind_inproc_2_)
            .SetNotaryName(notary_name_2_)
            .SetNotaryPublicPort(notary_public_port_2_)
            .SetNotaryTerms(notary_terms_2_)
            .SetStoragePlugin(storage_plugin_2_);
    const auto test3 = opentxs::Options{}.SetNotaryName(notary_name_3_);

    const auto expected1 = OptionsData{
        {},
        blockchain_storage_level_1_,
        blockchain_sync_enabled_1_,
        {},
        blockchain_wallet_enabled_1_,
        home_1_,
        log_endpoint_1_,
        log_leve_1_,
        notary_bind_inproc_1_,
        notary_bind_ip_1_,
        notary_bind_port_1_,
        notary_name_1_,
        {notary_public_eep_1_},
        {notary_public_ipv4_1_},
        {notary_public_ipv6_1_},
        {notary_public_opion_1_},
        notary_public_port_1_,
        notary_terms_1_,
        storage_plugin_1_};
    const auto expected2 = OptionsData{
        {blockchain_2_},
        blockchain_storage_level_2_,
        blockchain_sync_enabled_2_,
        {sync_server_2_},
        blockchain_wallet_enabled_2_,
        home_2_,
        log_endpoint_2_,
        log_leve_2_,
        notary_bind_inproc_2_,
        notary_bind_ip_2_,
        notary_bind_port_2_,
        notary_name_2_,
        {notary_public_eep_1_, notary_public_eep_2_},
        {notary_public_ipv4_1_, notary_public_ipv4_2_},
        {notary_public_ipv6_1_, notary_public_ipv6_2_},
        {notary_public_opion_1_, notary_public_opion_2_},
        notary_public_port_2_,
        notary_terms_2_,
        storage_plugin_2_};
    const auto expected3 = OptionsData{
        {blockchain_2_},
        blockchain_storage_level_2_,
        blockchain_sync_enabled_2_,
        {sync_server_2_},
        blockchain_wallet_enabled_2_,
        home_2_,
        log_endpoint_2_,
        log_leve_2_,
        notary_bind_inproc_2_,
        notary_bind_ip_2_,
        notary_bind_port_2_,
        notary_name_3_,
        {notary_public_eep_2_},
        {notary_public_ipv4_2_},
        {notary_public_ipv6_2_},
        {notary_public_opion_2_},
        notary_public_port_2_,
        notary_terms_2_,
        storage_plugin_2_};

    EXPECT_TRUE(check_options(test1 + blank, expected1));
    EXPECT_TRUE(check_options(blank + test1, expected1));
    EXPECT_TRUE(check_options(test1 + test2, expected2));
    EXPECT_TRUE(check_options(test2 + test3, expected3));
}
}  // namespace ottest
