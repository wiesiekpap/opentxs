// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/crypto/PaymentCodeV3.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

namespace ottest
{
auto GetPaymentCodeVector3() noexcept -> const PaymentCodeVectorsV3&
{
    // clang-format off
    static const auto data = PaymentCodeVectorsV3{
        "86f411ab1c8e70ae8a0795ab7a6757aea6e4d5ae1826fc7b8f00c597d500609c01000000",
        {"response seminar brave tip suit recall often sound stick owner lottery motion",
         "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
         {
             "77a215775bbacf7e0d325154a093e8d9f69c19f45b08d00114b214cc24b134f9",
             "f84e63e94e70678ab8367ef91711259fc98885be92479afec1a6a656e2245636",
             "05be1671949473c1b252db7aff98a8704841ad7cd19596f9d64ed81bd3e58bc8",
         },
         ot::blockchain::Type::Bitcoin,
         {
             "024edba30e70855e7846e850982f2eb3aefe33b292cc9a744604367de14cc018b8",
             "03a769eb57ce38dc3f7d80c4464bc61b02153a8e881c472d6d3e99b1d8fe53100c",
             "038f8e84682fb78ec6fdf3560020df035e144ce60bb9b09dd99b606d130140bd2c",
             "0210964b717a97430e9ca206bf84e1b0834385a03af3c749d60ad632d31e511954",
             "03b24c25099596f0984e4eedcc6147d1faff269a79f919e5d42414ea0691749174",
             "0285b4cda5356a7333510fac98fc27da4df8a3fcf6f50df594fbe6013e78d64114",
             "02a6946888b559db413f94a6de3aa974d4c22d881f132f753297baef510219327c",
             "02ab944a2509a27b9b9f569736e6cb45cb1c900627573a01bb9dffe38131103a12",
             "0276442e645c3f5e412b60ffac771a67b0ef1b652b18f18d101e9c6f70365cd183",
             "039386636f65cbc72a70bacc0f43ee17862ead8d37941e72b630f37e048ef2d405",
         },
         "872313fe1beb41a9e1ae19c0def97591e5c204387b64b85f4077078b232906d0",
         "0383b5e54776628baacee0cbb66b4db31aa95176dba1f62cabf0415103d0fdbda6",
         "0292d97c287932848852890ded442311623e32ebfeba12e2020b41c2fbe12f3812",
         "02ce75616fcd80345bca54dabd279b155f960c57260378455b872269221de231b6",
         "0292d97c287932848852890ded442311623e32ebfeba12e2020b41c2fbe12f3812"},
        {"reward upper indicate eight swift arch injury crystal super wrestle already dentist",
         "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
         {
             "4e299b083f610d5ab6e7e241089f185cf222deb9a790eacf01a72930c90d2261",
             "a6806034129abafba1511019991cca9bd8bededb1580bdc4fe0eb905dec8da2d",
             "ce75616fcd80345bca54dabd279b155f960c57260378455b872269221de231b6",
         },
         ot::blockchain::Type::Bitcoin_testnet3,
         {
             "03dc41458b939d966a0e141281c2a7c5faf184dc43bc26160f0ffc3c583600c9b6",
             "02513de274f78ce0c8cb827f25aae2ade941ac9d482002fc04ef60d580c5403afd",
             "033cf4391b3e7daad0220b572d796ac0711e93e2ef389119d3ec0bed2debf0472a",
             "02de639a0d80bc8b6976e71e5242b7f0ba5e9f8f6b317c0a180884424600bcaafc",
             "036b08a58e0d664505c95e2e0ceaa87e34c82cc6ed91a94980fc631967cc8d931f",
             "029d5dae4c27c59a9c207a1beafab9f1b8bef93e19b8bbd7614dae37e8f7c0210c",
             "03003668a8915ba65adb9ff8cfcce7f8d5aae2655a210e1e863eda6cb41dd5e1d2",
             "03a857d0bef97a0e5ffb1911e7cd13ced1bdce9c2a6a838dd5bdd8e805f44b8cc9",
             "02bcfcdc2e7fbdaebf1fa69a74ccd219c919981353433538ff98979c252609c564",
             "029dccbb87fec52713f90afbbef3e78dddef4dfa6858bbf2a5fd2fcd2582a5cf7f",
         },
         "0fb05a28df58b2add0d01eb491962b79092239e4d9396442eed83144b6541f4c",
         "0389087b9573ccc7efc5252a8a7c93d349d9b3dd882724c818e5369cbff0647d35",
         "027f88837a6d02949388c80c43efd352bea4483bb86764ba3dfa5b0c11e97b0ebc",
         "0205be1671949473c1b252db7aff98a8704841ad7cd19596f9d64ed81bd3e58bc8",
         "027f88837a6d02949388c80c43efd352bea4483bb86764ba3dfa5b0c11e97b0ebc"},
        {"response seminar brave tip suit recall often sound stick owner lottery motion",
         "PM8TJTLJbPRGxSbc8EJi42Wrr6QbNSaSSVJ5Y3E4pbCYiTHUskHg13935Ubb7q8tx9GVbh2UuRnBc3WSyJHhUrw8KhprKnn9eDznYGieTzFcwQRya4GA",
         {},
         ot::blockchain::Type::Bitcoin,
         {
             "0275fd0d1519ac24f4944a843c6475cda578d6286ee846ece105d5b0c69a1b9b3e",
             "035e6e2c1701229a7a4cddcfa83f42f06cafeba42a63dfb3287c35ffb8d1c6f872",
             "039f32033263f0fac7636fc3f2ffc68d4701dcb81641ed6c0f8d76b4fc7f2b5a3a",
             "02aa38313e99a2e7d8f22a7efa6b1c4b1f2769e4b91622bd4e6d919ffd2a41f388",
             "02ed75fc071317eae5d046389d54614dfb787856739f812e8de0f12f30d4d6bcee",
             "03d6cfc06f11b706a0f34de2486246c4495ce90ee02553d3239b5a235d19153ba3",
             "034b87613a9a6230ad831839e5da55d29970154d6b91bd3e42aeeb70b234b1b9ff",
             "02f014490536f79c0c8c1b2cee39428a1446a74ec1af19ce515da6e3fb29db0cfa",
             "03f96d069cffd4a0bb9430f6cda1376cee63825ec1619b5cc22077122c36b200c9",
             "03406c84ad2c5549af30e0c8886922eb51db925e35b7ed058d52f2458f1ce5d8ca",
         },
         "872313fe1beb41a9e1ae19c0def97591e5c204387b64b85f4077078b232906d0",
         "0383b5e54776628baacee0cbb66b4db31aa95176dba1f62cabf0415103d0fdbda6",
         "01000292d97c287932848852890ded442311623e32ebfeba12e2020b41c2fbe12f38127e36136254548e0a4dfc14232b6de263a69a1955629b75137d2beeccb639b2d300000000000000000000000000",
         "044e299b083f610d5ab6e7e241089f185cf222deb9a790eacf01a72930c90d226101000292d97c287932848852890ded442311623e32ebfeba12e2020b41c2fbe1",
         "042f38127e36136254548e0a4dfc14232b6de263a69a1955629b75137d2beeccb639b2d30000000000000000000000000000000000000000000000000000000000"},
        {"reward upper indicate eight swift arch injury crystal super wrestle already dentist",
         "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
         {
             "4e299b083f610d5ab6e7e241089f185cf222deb9a790eacf01a72930c90d2261",
             "a6806034129abafba1511019991cca9bd8bededb1580bdc4fe0eb905dec8da2d",
             "ce75616fcd80345bca54dabd279b155f960c57260378455b872269221de231b6",
         },
         ot::blockchain::Type::Bitcoin_testnet3,
         {
             "03bebccead98ce8d1ba065b814ce565ea8a79979a572c901188d5c274af1c82d9b",
             "02f0e0a7998e55c559fa57292a101cf5ab6e6cbe86bfdcf222e26d63c7773bbcf0",
             "038a2b1c008a9e048b2bb93d4eac24357012d5bd0fe84e772af646bad482f0528b",
             "0343cd902a36f5a4dda6bb11489f7bb4ebe381586e07fafc9e5039abb14c314972",
             "02b90e340e73eb2bc1addaf51e2229ec1d2aae78569879d022d208964fa741b6e1",
             "03f1ff5daadfb93ca15dfe4fe96ee77fa6465bcf0b98ab6b599e8b7a8e962ed38d",
             "02d7f5628abf1b64f7f4f1eea11a510740d1110a8f937821870eb5f47155b09c70",
             "02bd05db146de0273e704ecf162bef5de21032fc9b646237038d9f00004436172c",
             "020f0b8c3bf9a97566fae19b30b93b6a19b26905bcf6742260699a1cba39ff639d",
             "03ac995a811b20e68541ab1840671c84cc2765ed9625a53daf57c9f07ecfabfb42",
         },
         "0fb05a28df58b2add0d01eb491962b79092239e4d9396442eed83144b6541f4c",
         "0389087b9573ccc7efc5252a8a7c93d349d9b3dd882724c818e5369cbff0647d35",
         "030002b07dccadbf4289635ecf85eb12e1a0d7a7c93bcb905d5a5b2e75e9248fe4c92c113ccdbd406889022aa8a3d40d329553586d73914877626479f26b08ef7d3de000000000000000000000000000",
         "",
         ""},
    };
    // clang-format on

    return data;
}
}  // namespace ottest
