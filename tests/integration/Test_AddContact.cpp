// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <future>
#include <string>
#include <utility>

#include "integration/Helpers.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "ui/Helpers.hpp"

namespace opentxs
{
namespace api
{
namespace server
{
class Manager;
}  // namespace server
}  // namespace api
}  // namespace opentxs

namespace ottest
{
Counter contact_list_alex_{};
Counter messagable_list_alex_{};
Counter contact_list_bob_{};
Counter messagable_list_bob_{};
Counter contact_list_chris_{};
Counter messagable_list_chris_{};

struct Test_AddContact : public IntegrationFixture {
    const ot::api::client::Manager& api_alex_;
    const ot::api::client::Manager& api_bob_;
    const ot::api::client::Manager& api_chris_;
    const ot::api::server::Manager& api_server_1_;

    Test_AddContact()
        : api_alex_(ot::Context().StartClient(0))
        , api_bob_(ot::Context().StartClient(1))
        , api_chris_(ot::Context().StartClient(2))
        , api_server_1_(ot::Context().StartServer(0))
    {
        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(alex_).init(api_alex_, server_1_);
        const_cast<User&>(bob_).init(api_bob_, server_1_);
        const_cast<User&>(chris_).init(api_chris_, server_1_);
    }
};

TEST_F(Test_AddContact, init_ot) {}

TEST_F(Test_AddContact, init_ui)
{
    contact_list_alex_.expected_ = 1;
    contact_list_bob_.expected_ = 1;
    contact_list_chris_.expected_ = 1;
    messagable_list_alex_.expected_ = 0;
    messagable_list_bob_.expected_ = 0;
    messagable_list_chris_.expected_ = 0;

    init_contact_list(alex_, contact_list_alex_);
    init_contact_list(bob_, contact_list_bob_);
    init_contact_list(chris_, contact_list_chris_);
    init_messagable_list(alex_, messagable_list_alex_);
    init_messagable_list(bob_, messagable_list_bob_);
    init_messagable_list(chris_, messagable_list_chris_);
}

TEST_F(Test_AddContact, initial_state_contact_list_alex)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alex_));
    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_alex)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
    EXPECT_TRUE(check_messagable_list(alex_, expected));
    EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, initial_state_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, initial_state_contact_list_chris)
{
    ASSERT_TRUE(wait_for_counter(contact_list_chris_));

    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_chris_));
    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_chris)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
    EXPECT_TRUE(check_messagable_list(chris_, expected));
    EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, introduction_server)
{
    api_alex_.OTX().StartIntroductionServer(alex_.nym_id_);
    api_bob_.OTX().StartIntroductionServer(bob_.nym_id_);
    api_chris_.OTX().StartIntroductionServer(chris_.nym_id_);
    auto alexTask =
        api_alex_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true);
    auto bobTask =
        api_bob_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);
    auto chrisTask =
        api_chris_.OTX().RegisterNymPublic(chris_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, alexTask.first);
    ASSERT_NE(0, bobTask.first);
    ASSERT_NE(0, chrisTask.first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, alexTask.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, bobTask.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, chrisTask.second.get().first);

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();
}

TEST_F(Test_AddContact, nymid)
{
    ASSERT_FALSE(bob_.nym_id_->empty());

    contact_list_alex_.expected_ += 1;
    messagable_list_alex_.expected_ += 1;
    const auto& widget = alex_.api_->UI().ContactList(alex_.nym_id_);
    const auto id = widget.AddContact(bob_.name_, bob_.nym_id_->str(), "");
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(alex_.SetContact(bob_.name_, id));
}

TEST_F(Test_AddContact, add_nymid_contact_list_alex)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alex_));
    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, add_nymid_messagable_list_alex)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
    EXPECT_TRUE(check_messagable_list(alex_, expected));
    EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
TEST_F(Test_AddContact, paymentcode)
{
    ASSERT_FALSE(chris_.payment_code_.empty());

    contact_list_alex_.expected_ += 1;
    messagable_list_alex_.expected_ += 1;
    const auto& widget = alex_.api_->UI().ContactList(alex_.nym_id_);
    const auto id = widget.AddContact(chris_.name_, "", chris_.payment_code_);
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(alex_.SetContact(chris_.name_, id));
}

TEST_F(Test_AddContact, add_paymentcode_contact_list_alex)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alex_));
    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, add_paymentcode_messagable_list_alex)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
    EXPECT_TRUE(check_messagable_list(alex_, expected));
    EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

TEST_F(Test_AddContact, both)
{
    ASSERT_FALSE(alex_.nym_id_->empty());
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    ASSERT_FALSE(alex_.payment_code_.empty());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    contact_list_bob_.expected_ += 1;
    messagable_list_bob_.expected_ += 1;
    const auto& widget = bob_.api_->UI().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        alex_.name_, alex_.nym_id_->str(), alex_.payment_code_);
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(bob_.SetContact(alex_.name_, id));
}

TEST_F(Test_AddContact, add_both_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, add_both_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, backwards)
{
    ASSERT_FALSE(chris_.nym_id_->empty());
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    ASSERT_FALSE(chris_.payment_code_.empty());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    contact_list_bob_.expected_ += 1;
    messagable_list_bob_.expected_ += 1;
    const auto& widget = bob_.api_->UI().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        chris_.name_, chris_.payment_code_, chris_.nym_id_->str());
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(bob_.SetContact(chris_.name_, id));
}

TEST_F(Test_AddContact, add_backwards_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, add_backwards_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "A", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
TEST_F(Test_AddContact, paymentcode_as_nymid)
{
    ASSERT_FALSE(alex_.payment_code_.empty());

    contact_list_chris_.expected_ += 1;
    messagable_list_chris_.expected_ += 1;
    const auto& widget = chris_.api_->UI().ContactList(chris_.nym_id_);
    const auto id = widget.AddContact(alex_.name_, alex_.payment_code_, "");
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(chris_.SetContact(alex_.name_, id));
}

TEST_F(Test_AddContact, add_payment_code_as_nymid_contact_list_chris)
{
    ASSERT_TRUE(wait_for_counter(contact_list_chris_));

    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_chris_));
    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, add_payment_code_as_nymid_messagable_list_chris)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
    EXPECT_TRUE(check_messagable_list(chris_, expected));
    EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

TEST_F(Test_AddContact, nymid_as_paymentcode)
{
    ASSERT_FALSE(bob_.nym_id_->empty());

    contact_list_chris_.expected_ += 1;
    messagable_list_chris_.expected_ += 1;
    const auto& widget = chris_.api_->UI().ContactList(chris_.nym_id_);
    const auto id = widget.AddContact(bob_.name_, "", bob_.nym_id_->str());
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(chris_.SetContact(bob_.name_, id));
}

TEST_F(Test_AddContact, add_nymid_as_paymentcode_contact_list_chris)
{
    ASSERT_TRUE(wait_for_counter(contact_list_chris_));

    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
        {true, alex_.name_, alex_.name_, "A", ""},
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_chris_));
    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, add_nymid_as_paymentcode_messagable_list_chris)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

    const auto expected = ContactListData{{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
        {true, alex_.name_, alex_.name_, "A", ""},
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
    EXPECT_TRUE(check_messagable_list(chris_, expected));
    EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, shutdown)
{
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    EXPECT_EQ(contact_list_alex_.expected_, contact_list_alex_.updated_);
    EXPECT_EQ(messagable_list_alex_.expected_, messagable_list_alex_.updated_);
    EXPECT_EQ(contact_list_bob_.expected_, contact_list_bob_.updated_);
    EXPECT_EQ(messagable_list_bob_.expected_, messagable_list_bob_.updated_);
    EXPECT_EQ(contact_list_chris_.expected_, contact_list_chris_.updated_);
    EXPECT_EQ(
        messagable_list_chris_.expected_, messagable_list_chris_.updated_);
}
}  // namespace ottest
