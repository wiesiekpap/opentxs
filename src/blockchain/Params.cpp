// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/blockchain/Params.hpp"  // IWYU pragma: associated

#include <set>
#include <type_traits>

#if OT_BLOCKCHAIN
#include "opentxs/Bytes.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#endif  // OT_BLOCKCHAIN

#if OT_BLOCKCHAIN
namespace opentxs::blockchain
{
auto BlockHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256D, input, output);
        }
    }
}

auto FilterHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto MerkleHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto P2PMessageHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256DC, input, output);
        }
    }
}

auto ProofOfWorkHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Litecoin:
        case Type::Litecoin_testnet4: {
            return api.Crypto().Hash().Scrypt(
                input, input, 1024, 1, 1, 32, output);
        }
        case Type::UnitTest:
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::PKT:
        case Type::PKT_testnet:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto PubkeyHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_BITCOIN, input, output);
        }
    }
}

auto ScriptHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_BITCOIN, input, output);
        }
    }
}

auto SupportedChains() noexcept -> const std::set<Type>&
{
    static const auto build = []() -> auto
    {
        auto output = std::set<Type>{};

        for (const auto& [chain, data] : params::Data::chains_) {
            if (data.supported_) { output.emplace(chain); }
        }

        return output;
    };

    static const auto output{build()};

    return output;
}

auto TransactionHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::PKT:
        case Type::PKT_testnet:
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}
}  // namespace opentxs::blockchain
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::params
{
const Data::ChainData Data::chains_{
    {blockchain::Type::Bitcoin,
     {true,
      false,
      opentxs::proto::CITEMTYPE_BTC,
      "Bitcoin",
      "BTC",
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c",
      "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c01010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {630000,
       "6de9d737a62ea1c197000edb02c252089969dfd8ea4b02000000000000000000",
       "1e2b96b120a73bacb0667279bd231bdb95b08be16b650d000000000000000000",
       "63059e205633ebffb7d35e737611a6be5d1d3f904fa9c86a756afa7e0aee02f2"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      3652501241,
      8333,
      {
          "seed.bitcoin.sipa.be",
          "dnsseed.bluematt.me",
          "dnsseed.bitcoin.dashjr.org",
          "seed.bitcoinstats.com",
          "seed.bitcoin.jonasschnelli.ch",
          "seed.btc.petertodd.org",
          "seed.bitcoin.sprovoost.nl",
          "dnsseed.emzy.de",
      },
      25000,
      {{
          {u8"BTC", {"", u8"₿", {{10, 8}}, 0, 8}},
          {u8"mBTC", {"", u8"mBTC", {{10, 5}}, 0, 5}},
          {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
          {u8"μBTC", {"", u8"μBTC", {{10, 2}}, 0, 2}},
          {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
      }}}},
    {blockchain::Type::Bitcoin_testnet3,
     {true,
      true,
      opentxs::proto::CITEMTYPE_TNBTC,
      "Bitcoin (testnet3)",
      "tnBTC",
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae18",
      "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae1801010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {1775000,
       "ee03a42e71e08b0ec11f4a41b3dd49eb033a02d91c9e62607c00000000000000",
       "e34b7a89fd357ab76c784ad25dce9751aec2147407b520e9a21a9f2a00000000",
       "7a9e7964ffdf67007431fc592d3debd5e12a43dbf55a140ad51dfe5caae6ada1"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      118034699,
      18333,
      {
          "testnet-seed.bitcoin.jonasschnelli.ch",
          "seed.tbtc.petertodd.org",
          "seed.testnet.bitcoin.sprovoost.nl",
          "testnet-seed.bluematt.me",
          "testnet-seed.bitcoin.schildbach.de",
      },
      3113,
      {{
          {u8"BTC", {"", u8"tBTC", {{10, 8}}, 0, 8}},
          {u8"mBTC", {"", u8"mBTC", {{10, 5}}, 0, 5}},
          {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
          {u8"μBTC", {"", u8"μBTC", {{10, 2}}, 0, 2}},
          {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
      }}}},
    {blockchain::Type::BitcoinCash,
     {true,
      false,
      opentxs::proto::CITEMTYPE_BCH,
      "Bitcoin Cash",
      "BCH",
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c",
      "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c01010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {635259,
       "f73075b2c598f49b3a19558c070b52d5a5d6c21fefdf33000000000000000000",
       "1a78f5c89b05a56167066be9b0a36ac8f1781ed0472c30030000000000000000",
       "15ceed5cc33ecfdda722164b81b43e4e95ba7e1d65eaf94f7c2e3707343bf4c5"},
      filter::Type::Basic_BCHVariant,
      p2p::Protocol::bitcoin,
      3908297187,
      8333,
      {
          "seed.bitcoinabc.org",
          "seed-abc.bitcoinforks.org",
          "btccash-seeder.bitcoinunlimited.info",
          "seed.deadalnix.me",
          "seed.bchd.cash",
          "dnsseed.electroncash.de",
      },
      1000,
      {{
          {u8"BCH", {"", u8"BCH", {{10, 8}}, 0, 8}},
          {u8"mBCH", {"", u8"mBCH", {{10, 5}}, 0, 5}},
          {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
          {u8"μBCH", {"", u8"μBCH", {{10, 2}}, 0, 2}},
          {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
      }}}},
    {blockchain::Type::BitcoinCash_testnet3,
     {true,
      true,
      opentxs::proto::CITEMTYPE_TNBCH,
      "Bitcoin Cash (testnet3)",
      "tnBCH",
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae18",
      "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae1801010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {1378461,
       "d715e9fab7bbdf301081eeadbe6e931db282cf6b92b1365f9b50f59900000000",
       "24b33d026d36cbff3a693ea754a3be177dc5fb80966294cb643cf37000000000",
       "ee4ac1f50b0fba7dd9d2c5145fb4dd6a7e66b516c08a9b505cad8c2da53263fa"},
      filter::Type::Basic_BCHVariant,
      p2p::Protocol::bitcoin,
      4109624820,
      18333,
      {
          "testnet-seed.bitcoinabc.org",
          "testnet-seed-abc.bitcoinforks.org",
          "testnet-seed.bitprim.org",
          "testnet-seed.deadalnix.me",
          "testnet-seed.bchd.cash",
      },
      1000,
      {{
          {u8"BCH", {"", u8"tBCH", {{10, 8}}, 0, 8}},
          {u8"mBCH", {"", u8"mBCH", {{10, 5}}, 0, 5}},
          {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
          {u8"μBCH", {"", u8"μBCH", {{10, 2}}, 0, 2}},
          {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
      }}}},
    {blockchain::Type::Ethereum_frontier,
     {
         false,
         false,
         opentxs::proto::CITEMTYPE_ETH,
         "Ethereum (frontier)",
         "",
         0,
         "",
         "d4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3",
         "",
         {0, "", "", ""},
         {},
         p2p::Protocol::ethereum,
         0,
         30303,
         {},
         0,
         {}  // TODO
     }},
    {blockchain::Type::Ethereum_ropsten,
     {
         false,
         true,
         opentxs::proto::CITEMTYPE_ETHEREUM_ROPSTEN,
         "Ethereum (ropsten testnet)",
         "",
         0,
         "",
         "41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d",
         "",
         {0, "", "", ""},
         {},
         p2p::Protocol::ethereum,
         0,
         30303,
         {},
         0,
         {}  // TODO
     }},
    {blockchain::Type::Litecoin,
     {true,
      false,
      opentxs::proto::CITEMTYPE_LTC,
      "Litecoin",
      "LTC",
      504365040,  // 0x1e0ffff0
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97b9aa8e4e"
      "f0ff0f1ecd513f7c",
      "e2bf047e7e5a191aa4ef34d314979dc9986e0f19251edaba5940fd1fe365a712",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97b9aa8e4e"
      "f0ff0f1ecd513f7c01010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4804ffff001d0104404e592054696d65732030352f4f"
      "63742f32303131205374657665204a6f62732c204170706c65e280997320566973696f6e"
      "6172792c2044696573206174203536ffffffff0100f2052a010000004341040184710fa6"
      "89ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce60"
      "1aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9ac00000000",
      {1725000,
       "75086858fbea5c6782ff65a4eb5fa338e9d930d0320786abc707df00d30f41e3",
       "7014a9b68e409dddf51892d63f84183476268768a77fcbfb9ce11e4e28ad2610",
       "8bf7e9466b1be7a01a5106fe18b1402964e54fe843a92365f3f9012489b35859"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      3686187259,
      9333,
      {
          "seed-a.litecoin.loshan.co.uk",
          "dnsseed.thrasher.io",
          "dnsseed.litecointools.com",
          "dnsseed.litecoinpool.org",
      },
      25000,
      {{
          {u8"LTC", {"", u8"Ł", {{10, 8}}, 0, 6}},
          {u8"mLTC", {"", u8"mŁ", {{10, 5}}, 0, 3}},
          {u8"μLTC", {"", u8"μŁ", {{10, 2}}, 0, 0}},
          {u8"photons", {"", u8"photons", {{10, 2}}, 0, 0}},
      }}}},
    {blockchain::Type::Litecoin_testnet4,
     {true,
      true,
      opentxs::proto::CITEMTYPE_TNLTC,
      "Litecoin (testnet4)",
      "tnLTC",
      504365040,  // 0x1e0ffff0
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97f60ba158"
      "f0ff0f1ee1790400",
      "a0293e4eeb3da6e6f56f81ed595f57880d1a21569e13eefdd951284b5a626649",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97f60ba158"
      "f0ff0f1ee179040001010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4804ffff001d0104404e592054696d65732030352f4f"
      "63742f32303131205374657665204a6f62732c204170706c65e280997320566973696f6e"
      "6172792c2044696573206174203536ffffffff0100f2052a010000004341040184710fa6"
      "89ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce60"
      "1aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9ac00000000",
      {1550000,
       "8aef627831d5128695fcda8bc34b0d9780ad8f58d7a869f31f54af3acbbe7fff",
       "8295f3dcb3fea7b3536e6b8ed598c056edc6c5a23bc4efaf821086b56f24dda2",
       "2081dcdef3c741e20033a348a35d1b2952f31a9eee2285ea4f5cdb8c1c8decfe"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      4056470269,
      19335,
      {
          "testnet-seed.litecointools.com",
          "seed-b.litecoin.loshan.co.uk",
          "dnsseed-testnet.thrasher.io",
      },
      25000,
      {{
          {u8"LTC", {"", u8"Ł", {{10, 8}}, 0, 6}},
          {u8"mLTC", {"", u8"mŁ", {{10, 5}}, 0, 3}},
          {u8"μLTC", {"", u8"μŁ", {{10, 2}}, 0, 0}},
          {u8"photons", {"", u8"photons", {{10, 2}}, 0, 0}},
      }}}},
    {blockchain::Type::PKT,
     {true,
      true,  // TODO switch to false when testing is complete
      opentxs::proto::CITEMTYPE_PKT,
      "PKT",
      "PKT",
      521142271,  // 0x1f0fffff
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36ac891fb00000000"
      "ffff0f1f00000000",
      "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0b",
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36ac891fb00000000"
      "ffff0f1f0000000001fd04160000000000000000df345ba23b13467eec222a919d449dab"
      "6506abc555ef307794ecd3d36ac891fb00096e88ffff0f1f030000000000000034760700"
      "0098038000000000ffff0f20000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000e79d06f72d778459a76a989dbdded6d45b5e4358"
      "943c9aab1eb4e42a9c67f9ac317b762fe60198c3861255552928a179a5e9a6b9b7b7f4b4"
      "4e02fc3519f92964fbbfb576d1e9ff3c588c60fb2643602ae1f5695f89460608d3250e57"
      "a7755385aaa0de52409159387de4145d92533cd5f2a0d6d2a21b653311a40bd255649317"
      "1cf1beedf894a090626577d8042e72f9cdab8ab212b2d6ee5ca7b22169a01bf903ab05b2"
      "48fb8ed5de5a2bb0cd3901fc2e3270ffa524ed3adfc9d7fe109d0e2755f016386a09eda8"
      "1bd9707bf681d75cef829f3f8ee0903bfdb2c86ff44628df573143ec832f41ae17e575e3"
      "1848d9cf430930d81f41b0d803251b81f8181e543cb25c7dca4f2454f8f8bb86987db019"
      "ceffe7f0a2be807767f85dc903d3b843af448d14d5214b6ad5812b4d82b8cbea25c69c6b"
      "87d667f9c18c2993d500ed902d4c539a7d06ab0ca95afd946fd3702554e4bf9f76a1f087"
      "dccf33356b7efa9149fa6b4949159d03cb6e7d13efe91134a9ed8adc7c7325d39201cb2c"
      "2c1e2191c5e9d3d71dc5d1232e4cfc603fc5aa994d7bb8d190ca3d7c0e2fb9abb68df80c"
      "2cdfd8d119aec1a9c62c0ef7af9375e56c0330263332c4c879bcda52de73fea26781eb3d"
      "fa19dd2399b605050198fca80467bdca4a50980a3a37aa552f65caf9634b18fca475551d"
      "0a37dceab5f78c1cfdb48917122137cb74e236800c0684936b9cc0ca563025cb68609be3"
      "7869fa8e95bb6fdcd15320b3d5b2fabe9524f464dbfabe36ef958170b5d7f25c40938bd2"
      "87a5540b00e06ccb40f558958b72541e8ca4f4f965e4f78898085b34fdb6e33b1f588b6d"
      "0abc4cb119a8f54e0d949a08afb87979d4c69165ac6bd9e694369a3903ec24c1e3a52c40"
      "1c88e035a9f6aed6909f3a2b6dbe60e6fa842400c4164c21dc4c8b2325b70ad1829bed74"
      "2717776ff28457b384f4bdd0bf48b2db2d18f89af671c58ecded320cf289b8fa9cfd53fc"
      "d7352de1cff3c41d2f7f8ec6f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5"
      "e15b9b220b4209415537214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d0234a"
      "c95e535abd57d3ac91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb1451880271991"
      "e5f5db796ea8d392138cd18f602dc6deb3149c44e5085fbd77dc997571e4652009b55525"
      "3eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307"
      "b7ab0f3983f32f58b75fb02133f3e0778c089484d07058e76025855909ff64b7c2ace114"
      "b6c302a087acc140be90679fe1d0a75300573dc000000000ffff0f200000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000000000007594c5be"
      "146f727d7fb815193044fb2596ceca3a9b62252e5259ed56b7fb63cd2fe906fac0f3ff25"
      "658998198d9431a48a0be55a0a84333fbdabab0c318930b97d3bb1fa8a8ddeb1587f97c5"
      "31f81963c70784089465e2ef4f465b8d6bb9bbb27f36971c87b98ccae3f8d445181b03c9"
      "7a84ac8a12241b47d9845f966cedade1c31faa857bf2cafae9c71041dd23124d4cd4d6df"
      "f24cf632e94dd68831639b0f3aba27219cd8a869936605760ae408cadeef02c410fc2eeb"
      "412bdd7e411614e7830f54ebe0ea6eadae5fe226a67c0b310d4d4b5d10b47dfe2f165191"
      "e69c96e617ef8c3cf763fa49662deb82a2270b49816f11d56a3493c5e74b0eafbd9492e5"
      "fbaa0e0d6600c179a75c2c134e1d6a1c3721616b6241273b904aec0ef516c402649d032d"
      "5e4de8a1fb15bbeb250f5b6993b6bf5a39314e626d177578fedcc3f7935307321f8f25ae"
      "008855b1f19ddf26bcfa1636b3db132a737b4e1ec50ac9b223670f04a746be5c06e1de90"
      "115385c706af7eb947b9b712f9c14998d31b977ace19a1f2051799fe7aa47bc02f358f2d"
      "839891854825a7e7491e343eb5aa2d468e787989abf9961e21956c5ced5c6a5375e809ad"
      "958235fc91989fa4141230c42ccbf6a50c6ca61e3780d65dbfc112a104cc1da5b1dd7ea0"
      "24d2e37db0bb10ab6f06242589cb5383927ac5d130b189d32e4731ec1e8b675caf6c4da5"
      "31db3c598c5da69aa8ddcecae67cefd633fd80f994cec4ad28c2f1421b316999c1043c74"
      "9b14a645f785dd56e8fdbc959ff03648336b0c9c9ca3c86bb96738750b855dffa0b74c9c"
      "492580dcbbb892b91d76359aedc0a3d89a447b23f5449433bb7c4554eb6f0eb8ee63b9df"
      "12287f92eb23b3956d3933eeccf88ca9d9fe19a9a29a2821909f3a2b6dbe60e6fa842400"
      "c4164c21dc4c8b2325b70ad1829bed742717776ff28457b384f4bdd0bf48b2db2d18f89a"
      "f671c58ecded320cf289b8fa9cfd53fcd7352de1cff3c41d2f7f8ec6f280d8a9d6933da4"
      "2b66a6a3d30d46742e9cd793388a07e5e15b9b220b4209415537214447d386abce2c74a2"
      "4b7dc60ff9ce04a7cad19ab679d0234ac95e535abd57d3ac91747b2f2cfe1f01bb944502"
      "b827fc8d2c5e8f920fb1451880271991e5f5db796ea8d392138cd18f602dc6deb3149c44"
      "e5085fbd77dc997571e4652009b555253eefd215fb009b14e0e880f67d45e85a8252e457"
      "ddd0ace7cfdd5eec6cee070125b50307b7ab0f3983f32f58b75fb021ace16c1a11a478a7"
      "7f48ec8beda4f4912aa3337010343c14412cbc2f6d8ceb38dc88989cfee876ab00042a80"
      "00000000ffff0f2000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000009653aa497eb0bf1f7b9170967201419b6ced537def4363a0"
      "b2869d974a91d4458b4099f8d9a5f8555219c9b6efd193e1c745636d42cd705557c48e47"
      "598648c42e1c94318744855d037b3de60b626de12f06be4ec366527100b35ea8d4626eac"
      "5c2461d733c072811aa87bb5a39edf46d13a318f948367fe7a130359cd2a1ed04a60ee49"
      "7723623b258cecd2581a4d7cc3d7e9d05ae4d63ffcecdd16a19decb7dcffc9a9faccb208"
      "4177e736170f191b99446049304f95a2dad137670c0944a41dd36cd356ad70f65eaba467"
      "32e7976b4d252980db9e82ff554a599aae46dd27886e61a22adf51dbf26be34bbc766510"
      "ddebb15a9bef63ba3052fe7f71252807582e08fa1301fd78138917fec593f50758f10396"
      "6bcf45c32071a279367c90d2728d9d13a90c3ee64682b86b80738f4ad1cc94e8d2c98d70"
      "bc99e72b45a68f4719465bd291177ef8675eb9ab2cca7599bb8470180137e6d0e92dcd13"
      "fd60dfa8569175055e76d0df50c79447df8a0d6c64d1d240aae79168de62becc24097a5d"
      "a77de3d860efbf3fbb7a737275944899df27a45b9a7203d813dad5c6ebd0986535a26058"
      "9a51843ae43bf17902282439ce50ae75ab4ad8f994530750fc1b30d7dc364828b76275e3"
      "536950834c0afeb17ad04a0a3090cd4e1165b65727b08c939e355a5c992d87bd80c3a414"
      "65bf1b41d304646fbbfb6b350208282945b68d3a0440bb8d2dabf1b3767ccc02174499f4"
      "084be56f7733052ac65bec5401b9e627bb4094c8c5fad47a0afb5ab1a7db4de6e318f535"
      "013c8db58d16e5455fb0d2aa32a4d8e4d403412db7ecc718e459e81f09fde3523436ef61"
      "04f96201f1fa8c4251033198d39d0c5a87eae9b9499eb2b3551d4e579103de55354c95b4"
      "c3b0cee177cb443e85e4936100efb659bb7356a52f5d51682673e9cf655c9cec51d10097"
      "9ffbf74922dfeaecf1bf1ac55933c73d5f3fe927674fd5afc5d5a85e5b8d9779d7352de1"
      "cff3c41d2f7f8ec6f280d8a9d6933da42b66a6a3d30d46742e9cd793388a07e5e15b9b22"
      "0b4209415537214447d386abce2c74a24b7dc60ff9ce04a7cad19ab679d0234ac95e535a"
      "bd57d3ac91747b2f2cfe1f01bb944502b827fc8d2c5e8f920fb1451880271991e5f5db79"
      "6ea8d392138cd18f602dc6deb3149c44e5085fbd77dc997571e4652009b555253eefd215"
      "fb009b14e0e880f67d45e85a8252e457ddd0ace7cfdd5eec6cee070125b50307b7ab0f39"
      "83f32f58b75fb0213ab54f4815c5fb0803d5ddd6d4278fc7105e5a15aff36d31ba05dd09"
      "4c5d2b1f59974dd4d04c369300cb318000000000ffff0f20000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000d120d39a00a6aeb9"
      "703eaa6410db4990a504e21cdc0ccc4f913441b647104b4f0b8b87661db287ccaa443f29"
      "20759e0b9524babb4e227c7cc6a0ee765ff26b15ac81d3e764d6e4f8527edf236288ca56"
      "196d55a51a8c2a7cb9f9fd7f235a459fb9f77454c0a0cfbd71605850dcb3ad5428614ef5"
      "76b3cc358a2286bd7089a0459aea9c86741eb0e4e295ec976b94efcb4441e998c8e51758"
      "de78301ed490f799867355ecd7c57c1d6adfcb2f789f53f47ddd22fb6dad62b4d1b73150"
      "01c5b341a265587a38265e0e3ea811e53fbee01786efedc6bab28d0ece33016c96a7a52c"
      "c1c77cb8eb932020b883222dbb8a3c9209b7a8e9ef54828b205a63ce185fa813409d4589"
      "c203b782fae087f59141aca33b8a89af33314de4b215fb61821c03d76f0ac07d2d97e5ca"
      "d8fe5864de4269ddb23e0cbf4b53170a4b43da80e7d128f07a471f4ed7e81a9d4ab038cd"
      "4cb570c810bd4386b882b29d965824d651fdade58fa18a231a2ad288ed5fb0a1716c45c2"
      "4b80a332d5d8cd56d6f663b5b5bec1854bb2477b43bfa482d32577ebe6f775f1349c71fb"
      "98c49eccd2a6a984b29da8664e0715ce25b520e58622a207fd6f58b95a37b095308e2567"
      "2bca89d742faebbf8e397d5847a50266d4c8f76bdb9306d105a8a7d83d20ab07a8769fc1"
      "c64ae92233115a91352458a11f329b2b227b07e7aac5439354fd30e4c1ef22ed6061bdd6"
      "5020347eb495e40f7ed2d5e5dd6e6cbd34dcdb1078f771c3c93c8e2f989fd4af8e4704ac"
      "dae9f0a71e154bf6d0ada9efd1fc6a176299a3ef71fa650484d1d7062835a92def53df59"
      "6633bf39bf0383f30674ea81003187222c48d8d91989bfd41d40edde7b07c29f8da3e044"
      "6cc6f5c58f2941af4418658bc55c20dec60859c8e8f8545263179afdf5c1b48aedc0fb4b"
      "71bf00cd0e53e86d3af5350ba6ed0b283e2fbbe3333a2856b81f4db572f5193ef5c7561d"
      "d6c22e3c0b411fd711529e69bf05811b2e8ed4fcec0080b506394154245190535ebdf909"
      "fbaae9ced09b8f63f925e9170701598f9757e4db71546f4a4bbe4ad32be2f551f3841e31"
      "25881a4750ad6684076e0cf8a9565c3dfe5140b7b40f3578867a19cf652bef184f9ed2ad"
      "63bfa62e16bd8bb52232d76b171559acaa7c51d56103a83735f0d5b1ae3bc720e5085fbd"
      "77dc997571e4652009b555253eefd215fb009b14e0e880f67d45e85a8252e457ddd0ace7"
      "cfdd5eec6cee070125b50307b7ab0f3983f32f58b75fb0218228bfd8f3d022cd5a997867"
      "69f3a3e038e68fc7021fd54e8745ea09380d112f5846acb6b0b693a1ad015ae6d04e4311"
      "6192dc9edcdcdf52b2ece486afccac3a84da182bc48b69b3dec842c1d5f76abe2f9155a3"
      "22a03808f708af8b589bdd206c338a2fefa693bc9dc232bdb3c03d1fa32b1da8a4514de4"
      "fccb2df8c0ffa2036dc15a92cd13bcb938f3d76853db406ece5f3bbfc6adb556855af805"
      "acdf2b1784fba6e61c1288024f8609b9cee016f3b09c07b1e3257c03fc6f6a2bf40fd597"
      "d326d3eb2bb10c6a4412cb8e260153008a482f7315f2235a3ae044df7004944fddec3a3e"
      "ba0095fcb7432c07752f662e57559217925a030083452f8322f71a201497ceb1aa8efea8"
      "4504687932b1630f8440cd8b5b835424a99a6ba6ef531f0039c96dc9df6ddb1da17db619"
      "2d68265aa69fe8e7591d29f883799f4e8530085220cfe3d522c74c00ec447082de3f07f0"
      "3e4cf6f427b0f2e54fa73d0ee631d7e632101d487173ab63a5a014250a34f900730eb455"
      "4c4fcaff9e11e9051a3d7142d74708aadc2e29e3dec6fa67563527027c92a77e85f39702"
      "b90f869548e8d203f4b9166fd7ea1032e793228ea8ed223fa6d69ffef6c9ceca87df21a3"
      "3bf16d0095ccd7de5c20364a71f63933bc5e9f3269497e6bdc1969d6f4e2106a5ed1adcd"
      "971f9af95e595d00953c1527674ba6b82b0f8f6ce97ded33774c8defd97c5ff1efc54617"
      "984d68bde405e946062e16004f841e6d1cb21d25f844c947d9db391b6394537f0ee65b26"
      "70abcb51acb86515aa98155916420e00dadfa924a79604be0074b78bdba7439f6ac8a0b0"
      "28c43947f32cf1bde6af3dc9ffc3b36837c2e20083968aa01025b298c3f70f00028c0ed2"
      "71ba1f8a425d46a81e480ad932dce9f46a84d6ccfe205403ad32dc1b571683788d29b2db"
      "5a793410d9a5843fb29d60ab294e0ccc2f35bfe1593e112a44dd3408760054899838af83"
      "022b08c6b224b92da9961cf8e5c518c082f07b037c87f56d1c711e4564c8c3061b57767b"
      "6ffd2cb2f782d8a02db34ba0d94f6a0f8664af79fff0eac78b47b753df86cdb06ebe8801"
      "7a391df9656bf69eac1536d4237d19b601b632f65c35b264d0b634d17e2d8882af7cf585"
      "9b752801210e474f50eb15a8e67cb2be55332de8c389d1beeddfc275a3efeeb25ef6eadc"
      "57f4ab65436f7600d93cd72a0ee92af81941141ba58b6e361510f10bf66ff61ca2a3b6e0"
      "c83114d96bf382431fa21c00c9d818dc76721ed0ed09838560630ce2e2fc3ff2796727f0"
      "ded2147f68c040bf0b06c99184f0b53b13e966dd46b6224663f591dcb06be2c15398ad79"
      "af6155478d888c0cec4d0f008f0469a084a21a006ad610832938232cd672079fd672c29c"
      "fe44a9fe28029e4474b1d0efdf09ca6c99958969864e1a0483236c9a496f6753bd1dae21"
      "69f4a4a665d28907e5347aa30b181fa891a3d13c97612292424a7d21f89806e9ae3161be"
      "2e1067f7e5821c352cf985af08d990b2d5595dcf6aee29ba8f6a906990bb2407447e64dc"
      "31fdbb925dba728427683ef16e6fcde7b982390314a10cc5bd8c3a3fc9d4b1544a966301"
      "dbfda478712ea9de748ed1120bd864dab49694680dfdf647cb5d263d0a591c737fd38154"
      "75cbf0006bf0b638870865f9118936e144b4e7315763a5e526450325e1966ed32af3ec4f"
      "5c07231e161f4f006d0b61cd3a747951d29a6af505a27264206786b8de5339ea1972c7e1"
      "1027e77f90a5c9b11f5d2800490da63f1a94ffbb0bccc057f1be13eeae5cc8da783d3b84"
      "e2ae3aa424f54a663a4a9f9e67810f00b833ec0156377a6b96eb8b53e335f018af4b8be9"
      "4118485b2d3b53652e890526d1a41bded7141400a8cc33116507392c3db3dddf3291d975"
      "43c77e9a2c616dfe130f23d0bc3733b0f2843d32c51d0e04e7932ad21ec5e9be6dd6b86e"
      "541e2323ccf8b209ad0940b7222d4aaa91d8837fe42cf46b785af711ea8c6600320be68f"
      "cd657241e8efb16dde17e25f5adcf601aed934acfb3a82a2245a46f8b224527eb3ca48be"
      "ab1f052a044b9a7ef7d12a11c7e81bc72b0d3fce26f522a6180a762742d1e0ea79950a00"
      "0f653cf348876d1b2a42b4c7524dc906089023d96eff593c6eb9f0f4ecbd324800000101"
      "000000010000000000000000000000000000000000000000000000000000000000000000"
      "ffffffff0100ffffffff020000008011040000220020d5c1005c0d4012d3ae2672319e7f"
      "9eb15a57516aeefabbbc062265f67e308f2b0000000000000000326a3009f91102ffff0f"
      "20f935b3001ef51ba8f24921a404bc376a0c713274bd1cc68c2c57f66f5c0be7ca001000"
      "000000000000000000",
      {490000,
       "d6eb7a9f6de0c2cfddd2c08169ccc00110d831b406f28823fa4a423a0ff89b38",
       "428e5154459f4539a98e4d756da99eb98f8b181a2197c27ce4280b05ea227f76",
       "a79249bda2788c631c3acb80e2a57d68b821248b6ea3b6e93dd16f9b419c6d91"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      137298172,
      64764,
      {
          "seed.cjd.li",
          "seed.anode.co",
          "seed.gridfinity.com",
      },
      1000,
      {{
          {u8"PKT", {"", u8"PKT", {{2, 30}}, 0, 11}},
          {u8"mPKT", {"", u8"mPKT", {{2, 30}, {10, -3}}, 0, 8}},
          {u8"μPKT", {"", u8"μPKT", {{2, 30}, {10, -6}}, 0, 5}},
          {u8"nPKT", {"", u8"nPKT", {{2, 30}, {10, -9}}, 0, 2}},
          {u8"pack", {"", u8"pack", {{10, 0}}, 0, 2}},
      }}}},
    {blockchain::Type::PKT_testnet,
     {false,
      true,
      opentxs::proto::CITEMTYPE_TNPKT,
      "PKT (testnet)",
      "tnPKT",
      521142271,  // 0x1f0fffff
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "df345ba23b13467eec222a919d449dab6506abc555ef307794ecd3d36ac891fb00000000"
      "ffff0f1f00000000",
      "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0b",
      "TODO genesis block goes here",
      {0,
       "852d43936f4c9606a4a063cf356c454f2d9c43b07a41cf52e59461a41217dc0b",
       "0000000000000000000000000000000000000000000000000000000000000000",
       "526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36c"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      118034940,
      64764,
      {
          "testseed.cjd.li",
          "testseed.anode.co",
          "testseed.gridfinity.com",
      },
      1000,
      {{
          {u8"PKT", {"", u8"PKT", {{2, 30}}, 0, 11}},
          {u8"mPKT", {"", u8"mPKT", {{2, 30}, {10, -3}}, 0, 8}},
          {u8"μPKT", {"", u8"μPKT", {{2, 30}, {10, -6}}, 0, 5}},
          {u8"nPKT", {"", u8"nPKT", {{2, 30}, {10, -9}}, 0, 2}},
          {u8"pack", {"", u8"pack", {{10, 0}}, 0, 2}},
      }}}},
    {blockchain::Type::UnitTest,
     {false,
      true,
      opentxs::proto::CITEMTYPE_ERROR,
      "Unit Test Simulation",
      "UNITTEST",
      545259519,  // 0x207fffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff7f2002000000",
      "06226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910f",
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff7f200200000001010000000100000000000000000000000000000000000000000000"
      "00000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f"
      "4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f"
      "6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104"
      "678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f"
      "4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000",
      {0,
       "06226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910f",
       "0000000000000000000000000000000000000000000000000000000000000000",
       "18e0f10a66d223e36d0333dfe3e2546135f632f53e1e44ba016022f0a404628a"},
      filter::Type::Extended_opentxs,
      p2p::Protocol::bitcoin,
      3669344250,
      18444,
      {},
      0,
      {{
          {u8"Unit", {"", u8"units", {{10, 8}}, 0, 8}},
      }}}},
};

#if OT_BLOCKCHAIN
const Data::FilterData Data::genesis_filters_{
    {blockchain::Type::Bitcoin,
     {
         {filter::Type::Basic_BIP158,
          {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c202",
           "017fa880"}},
         {filter::Type::Extended_opentxs,
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99f",
           "049dc75e0d584a300293ef3d3980"}},
     }},
    {blockchain::Type::BitcoinCash,
     {
         {filter::Type::Basic_BCHVariant,
          {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c202",
           "017fa880"}},
         {filter::Type::Extended_opentxs,
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99f",
           "049dc75e0d584a300293ef3d3980"}},
     }},
    {blockchain::Type::Litecoin,
     {
         {filter::Type::Basic_BIP158,
          {"8aa75530308cf8247a151c37c24e7aaa281ae3b5cecedb581aacb3a0d07c2451",
           "019e8738"}},
         {filter::Type::Extended_opentxs,
          {"5a71cc36ad0b35d4c99b335ff69c3ed89e667b9772dbbf40396a1b4f2b2c3080",
           "049de8963322099e81f3bf7c4600"}},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {filter::Type::Basic_BIP158,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b9261315",
           "04e2f587e146bf6c662d35278a40"}},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {filter::Type::Basic_BCHVariant,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b9261315",
           "04e2f587e146bf6c662d35278a40"}},
     }},
    {blockchain::Type::Litecoin_testnet4,
     {
         {filter::Type::Basic_BIP158,
          {"02d023da9d271b849f717089aad7e03a515dac982c9fb2cfd952e2ce1c618792",
           "014c8c60"}},
         {filter::Type::Extended_opentxs,
          {"042bce138093a271d8d7f730f7f9f9ab8c7240f297b47aea4440dceec623aca3",
           "048b3d6095a4b01eb30ce44017c0"}},
     }},
    {blockchain::Type::PKT,
     {
         {filter::Type::Basic_BIP158,
          {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36c",
           "01902168"}},
         {filter::Type::Extended_opentxs,
          {"d8a96cd0b19de626e7d84c964cf1da0e09b90c589bd3e23bcf1c931779a47512",
           "02649a429ba06300"}},
     }},
    {blockchain::Type::PKT_testnet,
     {
         {filter::Type::Basic_BIP158,
          {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd36c",
           "01902168"}},
         {filter::Type::Extended_opentxs,
          {"d8a96cd0b19de626e7d84c964cf1da0e09b90c589bd3e23bcf1c931779a47512",
           "02649a429ba06300"}},
     }},
    {blockchain::Type::UnitTest,
     {
         {filter::Type::Basic_BIP158,
          {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e48",
           "014756c0"}},
         {filter::Type::Basic_BCHVariant,
          {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e48",
           "014756c0"}},
         {filter::Type::Extended_opentxs,
          {"18e0f10a66d223e36d0333dfe3e2546135f632f53e1e44ba016022f0a404628a",
           "042547f6de198130360443dfcdc0"}},
     }},
};

const Data::FilterTypes Data::bip158_types_{
    {Type::Bitcoin,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Bitcoin_testnet3,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::BitcoinCash,
     {
         {filter::Type::Basic_BCHVariant, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::BitcoinCash_testnet3,
     {
         {filter::Type::Basic_BCHVariant, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Ethereum_frontier, {}},
    {Type::Ethereum_ropsten, {}},
    {Type::Litecoin,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Litecoin_testnet4,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::PKT,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::PKT_testnet,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::UnitTest,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
};

const Data::ServiceBits Data::service_bits_{
    {blockchain::Type::Bitcoin,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::BitcoinCash,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
         {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
         {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
         {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
         {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
     }},
    {blockchain::Type::Litecoin,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::Litecoin_testnet4,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::PKT,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::PKT_testnet,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::UnitTest,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::params
