// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cctype>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"

namespace
{
class Test_BIP39 : public ::testing::Test
{
public:
    static constexpr auto type_{ot::crypto::SeedStyle::BIP39};
    static constexpr auto lang_{ot::crypto::Language::en};
    static std::set<std::string> generated_seeds_;

    const ot::api::client::Manager& api_;
    const ot::OTPasswordPrompt reason_;

    static auto word_count(const std::string& in) noexcept -> std::size_t
    {
        if (0 == in.size()) { return 0; }

        auto word = false;
        auto count = std::size_t{};

        for (const auto c : in) {
            if (std::isspace(c)) {
                if (word) {
                    word = false;
                } else {
                    continue;
                }
            } else {
                if (word) {
                    continue;
                } else {
                    word = true;
                    ++count;
                }
            }
        }

        return count;
    }

    auto generate_words(const ot::crypto::SeedStrength count) const
        -> std::size_t
    {
        const auto fingerprint =
            api_.Seeds().NewSeed(type_, lang_, count, reason_);

        EXPECT_EQ(generated_seeds_.count(fingerprint), 0);

        if (0 < generated_seeds_.count(fingerprint)) { return 0; }

        generated_seeds_.insert(fingerprint);

        const auto words = api_.Seeds().Words(reason_, fingerprint);

        return word_count(words);
    }

    Test_BIP39()
        : api_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
        , reason_(api_.Factory().PasswordPrompt(__FUNCTION__))
    {
    }
};

std::set<std::string> Test_BIP39::generated_seeds_{};

TEST_F(Test_BIP39, word_count)
{
    static const auto vector = std::map<std::string, std::size_t>{
        {"", 0},
        {" ", 0},
        {"     ", 0},
        {"one", 1},
        {" one", 1},
        {"   one", 1},
        {"one ", 1},
        {" one ", 1},
        {"   one   ", 1},
        {"one two", 2},
        {" one  two ", 2},
        {"   one   two ", 2},
        {"   one   two 3", 3},
    };

    for (const auto& [string, count] : vector) {
        EXPECT_EQ(word_count(string), count);
    }
}

TEST_F(Test_BIP39, longest_en)
{
    EXPECT_EQ(api_.Seeds().LongestWord(type_, lang_), 8);
}

TEST_F(Test_BIP39, twelve_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Twelve};
    constexpr auto words{12};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, fifteen_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Fifteen};
    constexpr auto words{15};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, eighteen_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Eighteen};
    constexpr auto words{18};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, twentyone_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyOne};
    constexpr auto words{21};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, twentyfour_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyFour};
    constexpr auto words{24};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, match_a)
{
    const auto test = std::string{"a"};
    const auto expected = std::vector<std::string>{
        "abandon", "ability",  "able",     "about",    "above",    "absent",
        "absorb",  "abstract", "absurd",   "abuse",    "access",   "accident",
        "account", "accuse",   "achieve",  "acid",     "acoustic", "acquire",
        "across",  "act",      "action",   "actor",    "actress",  "actual",
        "adapt",   "add",      "addict",   "address",  "adjust",   "admit",
        "adult",   "advance",  "advice",   "aerobic",  "affair",   "afford",
        "afraid",  "again",    "age",      "agent",    "agree",    "ahead",
        "aim",     "air",      "airport",  "aisle",    "alarm",    "album",
        "alcohol", "alert",    "alien",    "all",      "alley",    "allow",
        "almost",  "alone",    "alpha",    "already",  "also",     "alter",
        "always",  "amateur",  "amazing",  "among",    "amount",   "amused",
        "analyst", "anchor",   "ancient",  "anger",    "angle",    "angry",
        "animal",  "ankle",    "announce", "annual",   "another",  "answer",
        "antenna", "antique",  "anxiety",  "any",      "apart",    "apology",
        "appear",  "apple",    "approve",  "april",    "arch",     "arctic",
        "area",    "arena",    "argue",    "arm",      "armed",    "armor",
        "army",    "around",   "arrange",  "arrest",   "arrive",   "arrow",
        "art",     "artefact", "artist",   "artwork",  "ask",      "aspect",
        "assault", "asset",    "assist",   "assume",   "asthma",   "athlete",
        "atom",    "attack",   "attend",   "attitude", "attract",  "auction",
        "audit",   "august",   "aunt",     "author",   "auto",     "autumn",
        "average", "avocado",  "avoid",    "awake",    "aware",    "away",
        "awesome", "awful",    "awkward",  "axis",
    };
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_ar)
{
    const auto test = std::string{"ar"};
    const auto expected = std::vector<std::string>{
        "arch",
        "arctic",
        "area",
        "arena",
        "argue",
        "arm",
        "armed",
        "armor",
        "army",
        "around",
        "arrange",
        "arrest",
        "arrive",
        "arrow",
        "art",
        "artefact",
        "artist",
        "artwork"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arr)
{
    const auto test = std::string{"arr"};
    const auto expected =
        std::vector<std::string>{"arrange", "arrest", "arrive", "arrow"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arri)
{
    const auto test = std::string{"arri"};
    const auto expected = std::vector<std::string>{"arrive"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arrive)
{
    const auto test = std::string{"arrive"};
    const auto expected = std::vector<std::string>{"arrive"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arrived)
{
    const auto test = std::string{"arrived"};
    const auto expected = std::vector<std::string>{"arrive"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_axe)
{
    const auto test = std::string{"axe"};
    const auto expected = std::vector<std::string>{};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_empty_string)
{
    const auto test = std::string{""};
    const auto expected = std::vector<std::string>{};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}
}  // namespace
