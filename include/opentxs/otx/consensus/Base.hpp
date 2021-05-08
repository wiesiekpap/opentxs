// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_CONSENSUS_BASE_HPP
#define OPENTXS_OTX_CONSENSUS_BASE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>

#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"

namespace opentxs
{
namespace proto
{
class Context;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
namespace context
{
class OPENTXS_EXPORT Base : virtual public opentxs::contract::Signable
{
public:
    using TransactionNumbers = std::set<TransactionNumber>;
    using RequestNumbers = std::set<RequestNumber>;

    virtual RequestNumbers AcknowledgedNumbers() const = 0;
    virtual std::size_t AvailableNumbers() const = 0;
    virtual bool HaveLocalNymboxHash() const = 0;
    virtual bool HaveRemoteNymboxHash() const = 0;
    virtual TransactionNumbers IssuedNumbers() const = 0;
    virtual std::string LegacyDataFolder() const = 0;
    virtual OTIdentifier LocalNymboxHash() const = 0;
    virtual const identifier::Server& Notary() const = 0;
    virtual bool NymboxHashMatch() const = 0;
    virtual std::unique_ptr<const opentxs::NymFile> Nymfile(
        const PasswordPrompt& reason) const = 0;
    virtual const identity::Nym& RemoteNym() const = 0;
    virtual OTIdentifier RemoteNymboxHash() const = 0;
    virtual RequestNumber Request() const = 0;
    using Signable::Serialize;
    OPENTXS_NO_EXPORT virtual bool Serialize(proto::Context& out) const = 0;
    virtual otx::ConsensusType Type() const = 0;
    virtual bool VerifyAcknowledgedNumber(const RequestNumber& req) const = 0;
    virtual bool VerifyAvailableNumber(
        const TransactionNumber& number) const = 0;
    virtual bool VerifyIssuedNumber(const TransactionNumber& number) const = 0;

    virtual bool AddAcknowledgedNumber(const RequestNumber req) = 0;
    virtual bool CloseCronItem(const TransactionNumber) = 0;
    virtual bool ConsumeAvailable(const TransactionNumber& number) = 0;
    virtual bool ConsumeIssued(const TransactionNumber& number) = 0;
    virtual RequestNumber IncrementRequest() = 0;
    virtual bool InitializeNymbox(const PasswordPrompt& reason) = 0;
    virtual Editor<opentxs::NymFile> mutable_Nymfile(
        const PasswordPrompt& reason) = 0;
    virtual bool OpenCronItem(const TransactionNumber) = 0;
    virtual bool RecoverAvailableNumber(const TransactionNumber& number) = 0;
    virtual bool RemoveAcknowledgedNumber(const RequestNumbers& req) = 0;
    virtual void Reset() = 0;
    OPENTXS_NO_EXPORT virtual bool Refresh(
        proto::Context& out,
        const PasswordPrompt& reason) = 0;
    virtual void SetLocalNymboxHash(const Identifier& hash) = 0;
    virtual void SetRemoteNymboxHash(const Identifier& hash) = 0;
    virtual void SetRequest(const RequestNumber req) = 0;

    ~Base() override = default;

protected:
    Base() = default;

private:
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    Base& operator=(const Base&) = delete;
    Base& operator=(Base&&) = delete;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
#endif
