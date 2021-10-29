// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "internal/contact/Contact.hpp"  // IWYU pragma: associated

#include "opentxs/contact/Attribute.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::contact::internal
{
auto attribute_map() noexcept -> const AttributeMap&
{
    static const auto map = AttributeMap{
        {Attribute::Error, proto::CITEMATTR_ERROR},
        {Attribute::Active, proto::CITEMATTR_ACTIVE},
        {Attribute::Primary, proto::CITEMATTR_PRIMARY},
        {Attribute::Local, proto::CITEMATTR_LOCAL},
    };

    return map;
}

auto claimtype_map() noexcept -> const ClaimTypeMap&
{
    static const auto map = ClaimTypeMap{
        {ClaimType::Error, proto::CITEMTYPE_ERROR},
        {ClaimType::Individual, proto::CITEMTYPE_INDIVIDUAL},
        {ClaimType::Organization, proto::CITEMTYPE_ORGANIZATION},
        {ClaimType::Business, proto::CITEMTYPE_BUSINESS},
        {ClaimType::Government, proto::CITEMTYPE_GOVERNMENT},
        {ClaimType::Server, proto::CITEMTYPE_SERVER},
        {ClaimType::Prefix, proto::CITEMTYPE_PREFIX},
        {ClaimType::Forename, proto::CITEMTYPE_FORENAME},
        {ClaimType::Middlename, proto::CITEMTYPE_MIDDLENAME},
        {ClaimType::Surname, proto::CITEMTYPE_SURNAME},
        {ClaimType::Pedigree, proto::CITEMTYPE_PEDIGREE},
        {ClaimType::Suffix, proto::CITEMTYPE_SUFFIX},
        {ClaimType::Nickname, proto::CITEMTYPE_NICKNAME},
        {ClaimType::Commonname, proto::CITEMTYPE_COMMONNAME},
        {ClaimType::Passport, proto::CITEMTYPE_PASSPORT},
        {ClaimType::National, proto::CITEMTYPE_NATIONAL},
        {ClaimType::Provincial, proto::CITEMTYPE_PROVINCIAL},
        {ClaimType::Military, proto::CITEMTYPE_MILITARY},
        {ClaimType::PGP, proto::CITEMTYPE_PGP},
        {ClaimType::OTR, proto::CITEMTYPE_OTR},
        {ClaimType::SSL, proto::CITEMTYPE_SSL},
        {ClaimType::Physical, proto::CITEMTYPE_PHYSICAL},
        {ClaimType::Official, proto::CITEMTYPE_OFFICIAL},
        {ClaimType::Birthplace, proto::CITEMTYPE_BIRTHPLACE},
        {ClaimType::Home, proto::CITEMTYPE_HOME},
        {ClaimType::Website, proto::CITEMTYPE_WEBSITE},
        {ClaimType::Opentxs, proto::CITEMTYPE_OPENTXS},
        {ClaimType::Phone, proto::CITEMTYPE_PHONE},
        {ClaimType::Email, proto::CITEMTYPE_EMAIL},
        {ClaimType::Skype, proto::CITEMTYPE_SKYPE},
        {ClaimType::Wire, proto::CITEMTYPE_WIRE},
        {ClaimType::QQ, proto::CITEMTYPE_QQ},
        {ClaimType::Bitmessage, proto::CITEMTYPE_BITMESSAGE},
        {ClaimType::Whatsapp, proto::CITEMTYPE_WHATSAPP},
        {ClaimType::Telegram, proto::CITEMTYPE_TELEGRAM},
        {ClaimType::KIK, proto::CITEMTYPE_KIK},
        {ClaimType::BBM, proto::CITEMTYPE_BBM},
        {ClaimType::Wechat, proto::CITEMTYPE_WECHAT},
        {ClaimType::Kakaotalk, proto::CITEMTYPE_KAKAOTALK},
        {ClaimType::Facebook, proto::CITEMTYPE_FACEBOOK},
        {ClaimType::Google, proto::CITEMTYPE_GOOGLE},
        {ClaimType::Linkedin, proto::CITEMTYPE_LINKEDIN},
        {ClaimType::VK, proto::CITEMTYPE_VK},
        {ClaimType::Aboutme, proto::CITEMTYPE_ABOUTME},
        {ClaimType::Onename, proto::CITEMTYPE_ONENAME},
        {ClaimType::Twitter, proto::CITEMTYPE_TWITTER},
        {ClaimType::Medium, proto::CITEMTYPE_MEDIUM},
        {ClaimType::Tumblr, proto::CITEMTYPE_TUMBLR},
        {ClaimType::Yahoo, proto::CITEMTYPE_YAHOO},
        {ClaimType::Myspace, proto::CITEMTYPE_MYSPACE},
        {ClaimType::Meetup, proto::CITEMTYPE_MEETUP},
        {ClaimType::Reddit, proto::CITEMTYPE_REDDIT},
        {ClaimType::Hackernews, proto::CITEMTYPE_HACKERNEWS},
        {ClaimType::Wikipedia, proto::CITEMTYPE_WIKIPEDIA},
        {ClaimType::Angellist, proto::CITEMTYPE_ANGELLIST},
        {ClaimType::Github, proto::CITEMTYPE_GITHUB},
        {ClaimType::Bitbucket, proto::CITEMTYPE_BITBUCKET},
        {ClaimType::Youtube, proto::CITEMTYPE_YOUTUBE},
        {ClaimType::Vimeo, proto::CITEMTYPE_VIMEO},
        {ClaimType::Twitch, proto::CITEMTYPE_TWITCH},
        {ClaimType::Snapchat, proto::CITEMTYPE_SNAPCHAT},
        {ClaimType::Vine, proto::CITEMTYPE_VINE},
        {ClaimType::Instagram, proto::CITEMTYPE_INSTAGRAM},
        {ClaimType::Pinterest, proto::CITEMTYPE_PINTEREST},
        {ClaimType::Imgur, proto::CITEMTYPE_IMGUR},
        {ClaimType::Flickr, proto::CITEMTYPE_FLICKR},
        {ClaimType::Dribble, proto::CITEMTYPE_DRIBBLE},
        {ClaimType::Behance, proto::CITEMTYPE_BEHANCE},
        {ClaimType::Deviantart, proto::CITEMTYPE_DEVIANTART},
        {ClaimType::Spotify, proto::CITEMTYPE_SPOTIFY},
        {ClaimType::Itunes, proto::CITEMTYPE_ITUNES},
        {ClaimType::Soundcloud, proto::CITEMTYPE_SOUNDCLOUD},
        {ClaimType::Askfm, proto::CITEMTYPE_ASKFM},
        {ClaimType::Ebay, proto::CITEMTYPE_EBAY},
        {ClaimType::Etsy, proto::CITEMTYPE_ETSY},
        {ClaimType::Openbazaar, proto::CITEMTYPE_OPENBAZAAR},
        {ClaimType::Xboxlive, proto::CITEMTYPE_XBOXLIVE},
        {ClaimType::Playstation, proto::CITEMTYPE_PLAYSTATION},
        {ClaimType::Secondlife, proto::CITEMTYPE_SECONDLIFE},
        {ClaimType::Warcraft, proto::CITEMTYPE_WARCRAFT},
        {ClaimType::Alias, proto::CITEMTYPE_ALIAS},
        {ClaimType::Acquaintance, proto::CITEMTYPE_ACQUAINTANCE},
        {ClaimType::Friend, proto::CITEMTYPE_FRIEND},
        {ClaimType::Spouse, proto::CITEMTYPE_SPOUSE},
        {ClaimType::Sibling, proto::CITEMTYPE_SIBLING},
        {ClaimType::Member, proto::CITEMTYPE_MEMBER},
        {ClaimType::Colleague, proto::CITEMTYPE_COLLEAGUE},
        {ClaimType::Parent, proto::CITEMTYPE_PARENT},
        {ClaimType::Child, proto::CITEMTYPE_CHILD},
        {ClaimType::Employer, proto::CITEMTYPE_EMPLOYER},
        {ClaimType::Employee, proto::CITEMTYPE_EMPLOYEE},
        {ClaimType::Citizen, proto::CITEMTYPE_CITIZEN},
        {ClaimType::Photo, proto::CITEMTYPE_PHOTO},
        {ClaimType::Gender, proto::CITEMTYPE_GENDER},
        {ClaimType::Height, proto::CITEMTYPE_HEIGHT},
        {ClaimType::Weight, proto::CITEMTYPE_WEIGHT},
        {ClaimType::Hair, proto::CITEMTYPE_HAIR},
        {ClaimType::Eye, proto::CITEMTYPE_EYE},
        {ClaimType::Skin, proto::CITEMTYPE_SKIN},
        {ClaimType::Ethnicity, proto::CITEMTYPE_ETHNICITY},
        {ClaimType::Language, proto::CITEMTYPE_LANGUAGE},
        {ClaimType::Degree, proto::CITEMTYPE_DEGREE},
        {ClaimType::Certification, proto::CITEMTYPE_CERTIFICATION},
        {ClaimType::Title, proto::CITEMTYPE_TITLE},
        {ClaimType::Skill, proto::CITEMTYPE_SKILL},
        {ClaimType::Award, proto::CITEMTYPE_AWARD},
        {ClaimType::Likes, proto::CITEMTYPE_LIKES},
        {ClaimType::Sexual, proto::CITEMTYPE_SEXUAL},
        {ClaimType::Political, proto::CITEMTYPE_POLITICAL},
        {ClaimType::Religious, proto::CITEMTYPE_RELIGIOUS},
        {ClaimType::Birth, proto::CITEMTYPE_BIRTH},
        {ClaimType::Secondarygraduation, proto::CITEMTYPE_SECONDARYGRADUATION},
        {ClaimType::Universitygraduation,
         proto::CITEMTYPE_UNIVERSITYGRADUATION},
        {ClaimType::Wedding, proto::CITEMTYPE_WEDDING},
        {ClaimType::Accomplishment, proto::CITEMTYPE_ACCOMPLISHMENT},
        {ClaimType::BTC, proto::CITEMTYPE_BTC},
        {ClaimType::ETH, proto::CITEMTYPE_ETH},
        {ClaimType::XRP, proto::CITEMTYPE_XRP},
        {ClaimType::LTC, proto::CITEMTYPE_LTC},
        {ClaimType::DAO, proto::CITEMTYPE_DAO},
        {ClaimType::XEM, proto::CITEMTYPE_XEM},
        {ClaimType::DASH, proto::CITEMTYPE_DASH},
        {ClaimType::MAID, proto::CITEMTYPE_MAID},
        {ClaimType::LSK, proto::CITEMTYPE_LSK},
        {ClaimType::DOGE, proto::CITEMTYPE_DOGE},
        {ClaimType::DGD, proto::CITEMTYPE_DGD},
        {ClaimType::XMR, proto::CITEMTYPE_XMR},
        {ClaimType::WAVES, proto::CITEMTYPE_WAVES},
        {ClaimType::NXT, proto::CITEMTYPE_NXT},
        {ClaimType::SC, proto::CITEMTYPE_SC},
        {ClaimType::STEEM, proto::CITEMTYPE_STEEM},
        {ClaimType::AMP, proto::CITEMTYPE_AMP},
        {ClaimType::XLM, proto::CITEMTYPE_XLM},
        {ClaimType::FCT, proto::CITEMTYPE_FCT},
        {ClaimType::BTS, proto::CITEMTYPE_BTS},
        {ClaimType::USD, proto::CITEMTYPE_USD},
        {ClaimType::EUR, proto::CITEMTYPE_EUR},
        {ClaimType::GBP, proto::CITEMTYPE_GBP},
        {ClaimType::INR, proto::CITEMTYPE_INR},
        {ClaimType::AUD, proto::CITEMTYPE_AUD},
        {ClaimType::CAD, proto::CITEMTYPE_CAD},
        {ClaimType::SGD, proto::CITEMTYPE_SGD},
        {ClaimType::CHF, proto::CITEMTYPE_CHF},
        {ClaimType::MYR, proto::CITEMTYPE_MYR},
        {ClaimType::JPY, proto::CITEMTYPE_JPY},
        {ClaimType::CNY, proto::CITEMTYPE_CNY},
        {ClaimType::NZD, proto::CITEMTYPE_NZD},
        {ClaimType::THB, proto::CITEMTYPE_THB},
        {ClaimType::HUF, proto::CITEMTYPE_HUF},
        {ClaimType::AED, proto::CITEMTYPE_AED},
        {ClaimType::HKD, proto::CITEMTYPE_HKD},
        {ClaimType::MXN, proto::CITEMTYPE_MXN},
        {ClaimType::ZAR, proto::CITEMTYPE_ZAR},
        {ClaimType::PHP, proto::CITEMTYPE_PHP},
        {ClaimType::SEC, proto::CITEMTYPE_SEK},
        {ClaimType::TNBTC, proto::CITEMTYPE_TNBTC},
        {ClaimType::TNXRP, proto::CITEMTYPE_TNXRP},
        {ClaimType::TNLTX, proto::CITEMTYPE_TNLTC},
        {ClaimType::TNXEM, proto::CITEMTYPE_TNXEM},
        {ClaimType::TNDASH, proto::CITEMTYPE_TNDASH},
        {ClaimType::TNMAID, proto::CITEMTYPE_TNMAID},
        {ClaimType::TNLSK, proto::CITEMTYPE_TNLSK},
        {ClaimType::TNDOGE, proto::CITEMTYPE_TNDOGE},
        {ClaimType::TNXMR, proto::CITEMTYPE_TNXMR},
        {ClaimType::TNWAVES, proto::CITEMTYPE_TNWAVES},
        {ClaimType::TNNXT, proto::CITEMTYPE_TNNXT},
        {ClaimType::TNSC, proto::CITEMTYPE_TNSC},
        {ClaimType::TNSTEEM, proto::CITEMTYPE_TNSTEEM},
        {ClaimType::Philosophy, proto::CITEMTYPE_PHILOSOPHY},
        {ClaimType::Met, proto::CITEMTYPE_MET},
        {ClaimType::Fan, proto::CITEMTYPE_FAN},
        {ClaimType::Supervisor, proto::CITEMTYPE_SUPERVISOR},
        {ClaimType::Subordinate, proto::CITEMTYPE_SUBORDINATE},
        {ClaimType::Contact, proto::CITEMTYPE_CONTACT},
        {ClaimType::Refreshed, proto::CITEMTYPE_REFRESHED},
        {ClaimType::BOT, proto::CITEMTYPE_BOT},
        {ClaimType::BCH, proto::CITEMTYPE_BCH},
        {ClaimType::TNBCH, proto::CITEMTYPE_TNBCH},
        {ClaimType::Owner, proto::CITEMTYPE_OWNER},
        {ClaimType::Property, proto::CITEMTYPE_PROPERTY},
        {ClaimType::Unknown, proto::CITEMTYPE_UNKNOWN},
        {ClaimType::Ethereum_Olympic, proto::CITEMTYPE_ETHEREUM_OLYMPIC},
        {ClaimType::Ethereum_Classic, proto::CITEMTYPE_ETHEREUM_CLASSIC},
        {ClaimType::Ethereum_Expanse, proto::CITEMTYPE_ETHEREUM_EXPANSE},
        {ClaimType::Ethereum_Morden, proto::CITEMTYPE_ETHEREUM_MORDEN},
        {ClaimType::Ethereum_Ropsten, proto::CITEMTYPE_ETHEREUM_ROPSTEN},
        {ClaimType::Ethereum_Rinkeby, proto::CITEMTYPE_ETHEREUM_RINKEBY},
        {ClaimType::Ethereum_Kovan, proto::CITEMTYPE_ETHEREUM_KOVAN},
        {ClaimType::Ethereum_Sokol, proto::CITEMTYPE_ETHEREUM_SOKOL},
        {ClaimType::Ethereum_POA, proto::CITEMTYPE_ETHEREUM_POA},
        {ClaimType::PKT, proto::CITEMTYPE_PKT},
        {ClaimType::TNPKT, proto::CITEMTYPE_TNPKT},
        {ClaimType::Regtest, proto::CITEMTYPE_REGTEST},
    };

    return map;
}

auto sectiontype_map() noexcept -> const SectionTypeMap&
{
    static const auto map = SectionTypeMap{
        {SectionType::Error, proto::CONTACTSECTION_ERROR},
        {SectionType::Scope, proto::CONTACTSECTION_SCOPE},
        {SectionType::Identifier, proto::CONTACTSECTION_IDENTIFIER},
        {SectionType::Address, proto::CONTACTSECTION_ADDRESS},
        {SectionType::Communication, proto::CONTACTSECTION_COMMUNICATION},
        {SectionType::Profile, proto::CONTACTSECTION_PROFILE},
        {SectionType::Relationship, proto::CONTACTSECTION_RELATIONSHIP},
        {SectionType::Descriptor, proto::CONTACTSECTION_DESCRIPTOR},
        {SectionType::Event, proto::CONTACTSECTION_EVENT},
        {SectionType::Contract, proto::CONTACTSECTION_CONTRACT},
        {SectionType::Procedure, proto::CONTACTSECTION_PROCEDURE},
    };

    return map;
}

auto translate(const Attribute in) noexcept -> proto::ContactItemAttribute
{
    try {
        return attribute_map().at(in);
    } catch (...) {
        return proto::CITEMATTR_ERROR;
    }
}

auto translate(const ClaimType in) noexcept -> proto::ContactItemType
{
    try {
        return claimtype_map().at(in);
    } catch (...) {
        return proto::CITEMTYPE_ERROR;
    }
}

auto translate(const SectionType in) noexcept -> proto::ContactSectionName
{
    try {
        return sectiontype_map().at(in);
    } catch (...) {
        return proto::CONTACTSECTION_ERROR;
    }
}

auto translate(const proto::ContactItemAttribute in) noexcept -> Attribute
{
    static const auto map = reverse_arbitrary_map<
        contact::Attribute,
        proto::ContactItemAttribute,
        AttributeReverseMap>(attribute_map());

    try {
        return map.at(in);
    } catch (...) {
        return Attribute::Error;
    }
}

auto translate(const proto::ContactItemType in) noexcept -> ClaimType
{
    static const auto map = reverse_arbitrary_map<
        contact::ClaimType,
        proto::ContactItemType,
        ClaimTypeReverseMap>(claimtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return ClaimType::Error;
    }
}

auto translate(const proto::ContactSectionName in) noexcept -> SectionType
{
    static const auto map = reverse_arbitrary_map<
        contact::SectionType,
        proto::ContactSectionName,
        SectionTypeReverseMap>(sectiontype_map());

    try {
        return map.at(in);
    } catch (...) {
        return SectionType::Error;
    }
}
}  // namespace opentxs::contact::internal
