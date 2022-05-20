// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "identity/credential/Primary.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>

#include "Proto.hpp"
#include "core/contract/Signable.hpp"
#include "identity/credential/Key.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/Factory.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/asymmetric/Mode.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/SourceProofType.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/HDPath.pb.h"
#include "serialization/protobuf/MasterCredentialParameters.pb.h"
#include "serialization/protobuf/Signature.pb.h"
#include "serialization/protobuf/SourceProof.pb.h"
#include "util/Container.hpp"

namespace opentxs
{
auto Factory::PrimaryCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const crypto::Parameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
    -> identity::credential::internal::Primary*
{
    using ReturnType = identity::credential::implementation::Primary;

    try {

        return new ReturnType(api, parent, source, parameters, version, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to create credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::PrimaryCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const proto::Credential& serialized)
    -> identity::credential::internal::Primary*
{
    using ReturnType = identity::credential::implementation::Primary;

    try {

        return new ReturnType(api, parent, source, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to deserialize credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
const VersionConversionMap Primary::credential_to_master_params_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};

Primary::Primary(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const crypto::Parameters& params,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : credential::implementation::Key(
          api,
          parent,
          source,
          params,
          version,
          identity::CredentialRole::MasterKey,
          reason,
          "",
          identity::SourceType::PubKey == params.SourceType())
    , source_proof_(source_proof(params))
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init(*this, reason);
}

Primary::Primary(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const proto::Credential& serialized) noexcept(false)
    : credential::implementation::Key(api, parent, source, serialized, "")
    , source_proof_(serialized.masterdata().sourceproof())
{
    Lock lock(lock_);
    init_serialized(lock);
}

auto Primary::hasCapability(const NymCapability& capability) const -> bool
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            return signing_key_->CheckCapability(capability);
        }
        default: {
        }
    }

    return false;
}

auto Primary::Path(proto::HDPath& output) const -> bool
{
    try {
        const auto found = signing_key_->GetPrivateKey().Path(output);

        if (found) { output.mutable_child()->RemoveLast(); }

        return found;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("No private key.").Flush();

        return false;
    }
}

auto Primary::Path() const -> UnallocatedCString
{
    return signing_key_->GetPrivateKey().Path();
}

auto Primary::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<internal::Base::SerializedType>
{
    auto output = Key::serialize(lock, asPrivate, asSigned);

    OT_ASSERT(output);

    auto& serialized = *output;
    serialized.set_role(
        opentxs::translate(identity::CredentialRole::MasterKey));
    auto& masterData = *serialized.mutable_masterdata();
    masterData.set_version(credential_to_master_params_.at(version_));
    if (false == source_.Serialize(*masterData.mutable_source())) {
        throw std::runtime_error("Failed to serialize source.");
    }
    *masterData.mutable_sourceproof() = source_proof_;

    return output;
}

void Primary::sign(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason) noexcept(false)
{
    Key::sign(master, reason);

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE != source_proof_.type()) {
        auto sig = std::make_shared<proto::Signature>();

        OT_ASSERT(sig);

        if (false == source_.Sign(*this, *sig, reason)) {
            throw std::runtime_error("Failed to obtain source signature");
        }

        signatures_.push_back(sig);
    }
}

auto Primary::source_proof(const crypto::Parameters& params)
    -> proto::SourceProof
{
    auto output = proto::SourceProof{};
    output.set_version(1);
    output.set_type(translate(params.SourceProofType()));

    return output;
}

auto Primary::sourceprooftype_map() noexcept -> const SourceProofTypeMap&
{
    static const auto map = SourceProofTypeMap{
        {identity::SourceProofType::Error, proto::SOURCEPROOFTYPE_ERROR},
        {identity::SourceProofType::SelfSignature,
         proto::SOURCEPROOFTYPE_SELF_SIGNATURE},
        {identity::SourceProofType::Signature,
         proto::SOURCEPROOFTYPE_SIGNATURE},
    };

    return map;
}

auto Primary::translate(const identity::SourceProofType in) noexcept
    -> proto::SourceProofType
{
    try {
        return sourceprooftype_map().at(in);
    } catch (...) {
        return proto::SOURCEPROOFTYPE_ERROR;
    }
}

auto Primary::translate(const proto::SourceProofType in) noexcept
    -> identity::SourceProofType
{
    static const auto map = reverse_arbitrary_map<
        identity::SourceProofType,
        proto::SourceProofType,
        SourceProofTypeReverseMap>(sourceprooftype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::SourceProofType::Error;
    }
}

auto Primary::Verify(
    const proto::Credential& credential,
    const identity::CredentialRole& role,
    const Identifier& masterID,
    const proto::Signature& masterSig) const -> bool
{
    if (!proto::Validate<proto::Credential>(
            credential,
            VERBOSE,
            opentxs::translate(crypto::key::asymmetric::Mode::Public),
            opentxs::translate(role),
            false)) {
        LogError()(OT_PRETTY_CLASS())("Invalid credential syntax.").Flush();

        return false;
    }

    bool sameMaster = (id_ == masterID);

    if (!sameMaster) {
        LogError()(OT_PRETTY_CLASS())(
            "Credential does not designate this credential as its master.")
            .Flush();

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(credential);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(masterSig);
    signature.clear_signature();

    return Verify(api_.Factory().InternalSession().Data(copy), masterSig);
}

auto Primary::verify_against_source(const Lock& lock) const -> bool
{
    auto pSerialized = std::shared_ptr<proto::Credential>{};
    auto hasSourceSignature{true};

    switch (source_.Type()) {
        case identity::SourceType::PubKey: {
            pSerialized = serialize(lock, AS_PUBLIC, WITH_SIGNATURES);
            hasSourceSignature = false;
        } break;
        case identity::SourceType::Bip47: {
            pSerialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        } break;
        default: {
            return false;
        }
    }

    if (false == bool(pSerialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize credentials")
            .Flush();

        return false;
    }

    const auto& serialized = *pSerialized;
    const auto pSig = hasSourceSignature ? SourceSignature() : SelfSignature();

    if (false == bool(pSig)) {
        LogError()(OT_PRETTY_CLASS())(
            "Master credential not signed by its source.")
            .Flush();

        return false;
    }

    const auto& sig = *pSig;

    return source_.Verify(serialized, sig);
}

auto Primary::verify_internally(const Lock& lock) const -> bool
{
    // Perform common Key Credential verifications
    if (!Key::verify_internally(lock)) { return false; }

    // Check that the source validates this credential
    if (!verify_against_source(lock)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed verifying master credential against "
            "nym id source.")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::identity::credential::implementation
