// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"

namespace ottest
{
std::string profile_id_{};
std::string nym_id_{};
std::string server_id_{};

#define TEST_PASSWORD "blah foo blah foo blah"
#define TEST_DIFF_PASSWORD "time keeps on slippin slippin slippin"

class TestCallback : public opentxs::OTCallback
{
    std::string password_;

public:
    void runOne(
        const char* szDisplay,
        opentxs::Secret& theOutput,
        const std::string& key) const override
    {
        theOutput.AssignText(password_);
    }

    void runTwo(
        const char* szDisplay,
        opentxs::Secret& theOutput,
        const std::string& key) const override
    {
        theOutput.AssignText(password_);
    }

    void SetPassword(const std::string& password) { password_ = password; }

    TestCallback();
};

TestCallback::TestCallback()
    : opentxs::OTCallback()
    , password_()
{
}

TEST(PasswordCallback, create)
{
    opentxs::OTCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    profile_id_ = otx.ProfileId();
    const auto& client = otx.StartClient(0);
    const auto reason = client.Factory().PasswordPrompt(__func__);
    const auto nym = client.Wallet().Nym(reason);

    ASSERT_TRUE(nym);
    nym_id_ = nym->ID().str();
    EXPECT_FALSE(nym_id_.empty());

    ot::Cleanup();
}

TEST(PasswordCallback, load)
{
    opentxs::OTCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    const auto profile_id = otx.ProfileId();
    EXPECT_EQ(profile_id, profile_id_);

    const auto& client = otx.StartClient(0);
    const auto nym_identifier{opentxs::identifier::Nym::Factory(nym_id_)};
    const auto nym = client.Wallet().Nym(nym_identifier);

    ASSERT_TRUE(nym);
    const auto nym_id = nym->ID().str();
    EXPECT_FALSE(nym_id.empty());
    EXPECT_EQ(nym_id, nym_id_);

    // Have the Nym sign something here, which should succeed.

    auto reason = client.Factory().PasswordPrompt(__func__);
    auto message{client.Factory().Message()};

    const auto signed_success = message->SignContract(*nym, reason);
    ASSERT_TRUE(signed_success);

    ot::Cleanup();
}

TEST(PasswordCallback, wrongpw)
{
    opentxs::OTCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_DIFF_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    const auto& client = otx.StartClient(0);
    const auto nym_identifier{opentxs::identifier::Nym::Factory(nym_id_)};
    const auto nym = client.Wallet().Nym(nym_identifier);

    ASSERT_TRUE(nym);

    // Have the Nym sign something here, which should fail since
    // we deliberately used the wrong password.

    auto reason = client.Factory().PasswordPrompt(__func__);
    auto message{client.Factory().Message()};

    const auto signed_success = message->SignContract(*nym, reason);
    ASSERT_FALSE(signed_success);

    ot::Cleanup();
}
}  // namespace ottest
