// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

#include "Helpers.hpp"
#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/protobuf/BitcoinBlockHeaderFields.pb.h"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "opentxs/protobuf/BlockchainBlockLocalData.pb.h"

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::client;

namespace
{
class Test_BlockHeader : public ::testing::Test
{
public:
    const ot::api::client::internal::Manager& api_;

    Test_BlockHeader()
        : api_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient(OTTestEnvironment::test_args_, 0)))
    {
    }
};

TEST_F(Test_BlockHeader, init_opentxs) {}

TEST_F(Test_BlockHeader, btc_genesis_block_hash_oracle)
{
    const auto expectedHash =
        ot::Data::Factory(btc_genesis_hash_, ot::Data::Mode::Hex);
    const auto& genesisHash =
        bc::HeaderOracle::GenesisBlockHash(b::Type::Bitcoin);

    EXPECT_EQ(expectedHash.get(), genesisHash);
}

TEST_F(Test_BlockHeader, ltc_genesis_block_hash_oracle)
{
    const auto expectedHash =
        ot::Data::Factory(ltc_genesis_hash_, ot::Data::Mode::Hex);
    const auto& genesisHash =
        bc::HeaderOracle::GenesisBlockHash(b::Type::Litecoin);

    EXPECT_EQ(expectedHash.get(), genesisHash);
}

TEST_F(Test_BlockHeader, btc_genesis_block_header)
{
    const auto blankHash =
        ot::Data::Factory(std::string(blank_hash_), ot::Data::Mode::Hex);
    const auto expectedHash =
        ot::Data::Factory(btc_genesis_hash_, ot::Data::Mode::Hex);
    const std::string numericHash{btc_genesis_hash_numeric_};
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto& header = *pHeader;

    EXPECT_EQ(header.EffectiveState(), bb::Header::Status::Normal);
    EXPECT_EQ(expectedHash.get(), header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(header.InheritedState(), bb::Header::Status::Normal);
    EXPECT_FALSE(header.IsBlacklisted());
    EXPECT_FALSE(header.IsDisconnected());
    EXPECT_EQ(header.LocalState(), bb::Header::Status::Checkpoint);
    EXPECT_EQ(numericHash, header.NumericHash()->asHex());
    EXPECT_EQ(header.ParentHash(), blankHash.get());

    const auto [height, hash] = header.Position();

    EXPECT_EQ(header.Hash(), hash.get());
    EXPECT_EQ(header.Height(), height);
}

TEST_F(Test_BlockHeader, ltc_genesis_block_header)
{
    const auto blankHash =
        ot::Data::Factory(std::string(blank_hash_), ot::Data::Mode::Hex);
    const auto expectedHash =
        ot::Data::Factory(ltc_genesis_hash_, ot::Data::Mode::Hex);
    const std::string numericHash{ltc_genesis_hash_numeric_};
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Litecoin)};

    ASSERT_TRUE(pHeader);

    const auto& header = *pHeader;

    EXPECT_EQ(header.EffectiveState(), bb::Header::Status::Normal);
    EXPECT_EQ(expectedHash.get(), header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(header.InheritedState(), bb::Header::Status::Normal);
    EXPECT_FALSE(header.IsBlacklisted());
    EXPECT_FALSE(header.IsDisconnected());
    EXPECT_EQ(header.LocalState(), bb::Header::Status::Checkpoint);
    EXPECT_EQ(numericHash, header.NumericHash()->asHex());
    EXPECT_EQ(header.ParentHash(), blankHash.get());

    const auto [height, hash] = header.Position();

    EXPECT_EQ(header.Hash(), hash.get());
    EXPECT_EQ(header.Height(), height);
}

TEST_F(Test_BlockHeader, Serialize)
{
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto& header = *pHeader;
    const auto serialized = header.Serialize();
    const auto& local = serialized.local();
    const auto& bitcoin = serialized.bitcoin();

    EXPECT_EQ(serialized.version(), 1);
    EXPECT_EQ(serialized.type(), static_cast<std::uint32_t>(b::Type::Bitcoin));
    EXPECT_EQ(local.version(), 1);
    EXPECT_EQ(local.height(), header.Height());
    EXPECT_EQ(local.status(), static_cast<std::uint32_t>(header.LocalState()));
    EXPECT_EQ(
        local.inherit_status(),
        static_cast<std::uint32_t>(header.InheritedState()));
    EXPECT_EQ(bitcoin.version(), 1);
    EXPECT_EQ(bitcoin.previous_header(), header.ParentHash().str());
}

TEST_F(Test_BlockHeader, Deserialize)
{
    const auto expectedHash =
        ot::Data::Factory(btc_genesis_hash_, ot::Data::Mode::Hex);
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto serialized = pHeader->Serialize();

    pHeader = api_.Factory().BlockHeader(serialized);

    ASSERT_TRUE(pHeader);
    EXPECT_EQ(expectedHash.get(), pHeader->Hash());
}
}  // namespace
