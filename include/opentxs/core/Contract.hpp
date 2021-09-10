// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#ifndef OPENTXS_CORE_CONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"

namespace irr
{
namespace io
{
class IFileReadCallBack;
class IXMLBase;
template <class char_type, class super_class>
class IIrrXMLReader;

using IrrXMLReader = IIrrXMLReader<char, IXMLBase>;
}  // namespace io
}  // namespace irr

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

namespace identity
{
class Nym;
}  // namespace identity

class Armored;
class PasswordPrompt;
class Tag;
}  // namespace opentxs

namespace opentxs
{
using listOfSignatures = std::list<OTSignature>;

auto trim(const String& str) -> OTString;

class OPENTXS_EXPORT Contract
{
public:
    /** Used by OTTransactionType::Factory and OTToken::Factory. In both cases,
     * it takes the input string, trims it, and if it's armored, it unarmors it,
     * with the result going into strOutput. On success, bool is returned, and
     * strFirstLine contains the first line from strOutput. */
    static auto DearmorAndTrim(
        const String& strInput,
        String& strOutput,
        String& strFirstLine) -> bool;

    /** The Method "RewriteContract" calls this. I put the meat into a static
     * method so I could use it from outside Contract as well. */
    static auto AddBookendsAroundContent(
        String& strOutput,
        const String& strContents,
        const String& strContractType,
        const crypto::HashType hashType,
        const listOfSignatures& listSignatures) -> bool;
    static auto LoadEncodedTextField(
        irr::io::IrrXMLReader*& xml,
        Armored& ascOutput) -> bool;
    static auto LoadEncodedTextField(
        irr::io::IrrXMLReader*& xml,
        String& strOutput) -> bool;
    static auto LoadEncodedTextFieldByName(
        irr::io::IrrXMLReader*& xml,
        Armored& ascOutput,
        const char* szName,
        String::Map* pmapExtraVars = nullptr) -> bool;
    static auto LoadEncodedTextFieldByName(
        irr::io::IrrXMLReader*& xml,
        String& strOutput,
        const char* szName,
        String::Map* pmapExtraVars = nullptr) -> bool;
    static auto SkipToElement(irr::io::IrrXMLReader*& xml) -> bool;
    static auto SkipToTextField(irr::io::IrrXMLReader*& xml) -> bool;
    static auto SkipAfterLoadingField(irr::io::IrrXMLReader*& xml) -> bool;

    void SetIdentifier(const Identifier& theID);

    // TODO: a contract needs to have certain required fields in order to be
    // accepted for notarization. One of those should be a URL where anyone can
    // see a list of the approved e-notary servers, signed by the issuer.
    //
    // Why is this important?
    //
    // Because when the issuer connects to the e-notary to issue the currency,
    // he must upload the asset contract as part of that process. During the
    // same process, the e-notary connects to that standard URL and downloads a
    // RECORD, signed by the ISSUER, showing the e-notary on the accepted list
    // of transaction providers.
    //
    // Now the e-notary can make THAT record available to its clients (most
    // likely demanded by their wallet software) as proof that the issuer has,
    // in fact, issued digital assets on the e-notary server in question. This
    // provides proof that the issuer is, in fact, legally on the line for
    // whatever assets they have actually issued through that e-notary. The
    // issuer can make the total outstanding units available publicly, which
    // wallets can cross-reference with the public records on the transaction
    // servers. (The figures concerning total issued currency should match.)
    //
    // Of course, the transaction server could still lie, and publish a
    // falsified number instead of the actual total issued currency for a given
    // digital asset. Only systems can prevent that, based around separation of
    // powers. People will be more likely to trust the transaction provider who
    // has good accounting and code audit processes, with code fingerprints,
    // multiple passwords across neutral and bonded 3rd parties, insured, etc.
    // Ultimately these practices will be governed by the cost of insurance.
    //
    // But there WILL be winners who arise because they implement systems that
    // provide trust. And trust is a currency.
    //
    // (Currently the code loads the key FROM the contract itself, which won't
    // be possible when the issuer and transaction provider are two separate
    // entities. So this sort of protocol becomes necessary.)

    virtual ~Contract();
    virtual void Release();
    void Release_Contract();
    void ReleaseSignatures();

    /** This function is for those times when you already have the unsigned
     * version of the contract, and you have the signer, and you just want to
     * sign it and calculate its new ID from the finished result. */
    virtual auto CreateContract(
        const String& strContract,
        const identity::Nym& theSigner,
        const PasswordPrompt& reason) -> bool;

    /** CreateContract is great if you already know what kind of contract to
     * instantiate and have already done so. Otherwise this function will take
     * ANY flat text and use a generic Contract instance to sign it and then
     * write it to strOutput. This is due to the fact that OT was never really
     * designed for signing flat text, only contracts. */
    static auto SignFlatText(
        const api::Core& api,
        String& strFlatText,
        const String& strContractType,  // "LEDGER" or "PURSE" etc.
        const identity::Nym& theSigner,
        String& strOutput,
        const PasswordPrompt& reason) -> bool;
    inline void GetName(String& strName) const
    {
        strName.Set(m_strName->Get());
    }
    inline void SetName(const String& strName) { m_strName = strName; }
    inline auto GetContractType() const -> const String&
    {
        return m_strContractType;
    }
    /** This function calls VerifyContractID, and if that checks out, then it
     * looks up the official "contract" key inside the contract by calling
     * GetContractPublicNym, and uses it to verify the signature on the
     * contract. So the contract is self-verifying. Right now only public keys
     * are supported, but soon contracts will also support x509 certs. */
    virtual auto VerifyContract() const -> bool;

    /** Overriden for example in OTOffer, OTMarket. You can get it in string or
     * binary form. */
    virtual void GetIdentifier(Identifier& theIdentifier) const;

    /** The Contract ID is a hash of the contract raw file. */
    void GetIdentifier(String& theIdentifier) const;
    void GetFilename(String& strFilename) const;

    /** assumes m_strFilename is already set. Then it reads that file into a
     * string. Then it parses that string into the object. */
    virtual auto LoadContract() -> bool;
    auto LoadContract(const char* szFoldername, const char* szFilename) -> bool;

    /** Just like it says. If you have a contract in string form, pass it in
     * here to import it. */
    virtual auto LoadContractFromString(const String& theStr) -> bool;

    /** fopens m_strFilename and reads it off the disk into m_strRawFile */
    auto LoadContractRawFile() -> bool;

    /** data_folder/contracts/Contract-ID */
    auto SaveToContractFolder() -> bool;

    /** Saves the raw (pre-existing) contract text to any string you want to
     * pass in. */
    auto SaveContractRaw(String& strOutput) const -> bool;

    /** Takes the pre-existing XML contents (WITHOUT signatures) and re-writes
     * the Raw data, adding the pre-existing signatures along with new signature
     * bookends. */
    auto RewriteContract(String& strOutput) const -> bool;

    /** This saves the Contract to its own internal member string, m_strRawFile
     * (and does NOT actually save it to a file.) */
    virtual auto SaveContract() -> bool;

    /** Saves the contract to a specific filename */
    virtual auto SaveContract(const char* szFoldername, const char* szFilename)
        -> bool;

    /** Writes the contract to a specific filename without changing member
     *  variables */
    auto WriteContract(const std::string& folder, const std::string& filename)
        const -> bool;

    /** Update the internal unsigned contents based on the member variables
     * default behavior does nothing. */
    virtual void UpdateContents(const PasswordPrompt& reason);

    /** Only used when first generating an asset or server contract. Meant for
     * contracts which never change after that point. Otherwise does the same
     * thing as UpdateContents. (But meant for a different purpose.) */
    virtual void CreateContents();

    /** Overrides of CreateContents call this in order to add some common
     * internals. */
    void CreateInnerContents(Tag& parent);

    /** Save the internal contents (m_xmlUnsigned) to an already-open file */
    virtual auto SaveContents(std::ofstream& ofs) const -> bool;

    /** Saves the entire contract to a file that's already open (like a wallet).
     */
    virtual auto SaveContractWallet(Tag& parent) const -> bool;

    virtual auto DisplayStatistics(String& strContents) const -> bool;

    /** Save m_xmlUnsigned to a string that's passed in */
    virtual auto SaveContents(String& strContents) const -> bool;
    virtual auto SignContract(
        const identity::Nym& theNym,
        const PasswordPrompt& reason) -> bool;
    auto SignContractAuthent(
        const identity::Nym& theNym,
        const PasswordPrompt& reason) -> bool;
    auto SignWithKey(
        const crypto::key::Asymmetric& theKey,
        const PasswordPrompt& reason) -> bool;
    auto SignContract(
        const identity::Nym& theNym,
        Signature& theSignature,
        const PasswordPrompt& reason) -> bool;

    /** Uses authentication key instead of signing key. */
    auto SignContractAuthent(
        const identity::Nym& theNym,
        Signature& theSignature,
        const PasswordPrompt& reason) -> bool;
    auto SignContract(
        const crypto::key::Asymmetric& theKey,
        Signature& theSignature,
        const crypto::HashType hashType,
        const PasswordPrompt& reason) -> bool;

    /** Calculates a hash of m_strRawFile (the xml portion of the contract plus
    the signatures) and compares to m_ID (supposedly the same. The ID is
    calculated by hashing the file.)

    Be careful here--asset contracts and server contracts can have this ID. But
    a class such as OTAccount will change in its datafile as the balance
    changes. Thus, the account must have a Unique ID that is NOT a hash of its
    file.

    This means it's important to have the ID function overridable for
    OTAccount... This also means that my wallet MUST be signed, and these files
    should have and encryption option also. Because if someone changes my
    account ID in the file, I have no way of re-calculating it from the account
    file, which changes! So my copies of the account file and wallet file are
    the only records of that account ID which is a giant std::int64_t number. */
    virtual auto VerifyContractID() const -> bool;
    virtual void CalculateContractID(Identifier& newID) const;
    virtual void CalculateAndSetContractID(Identifier& newID);

    /** So far not overridden anywhere (used to be OTTrade.) */
    virtual auto VerifySignature(const identity::Nym& theNym) const -> bool;
    virtual auto VerifySigAuthent(const identity::Nym& theNym) const -> bool;
    virtual auto VerifyWithKey(const crypto::key::Asymmetric& theKey) const
        -> bool;
    auto VerifySignature(
        const identity::Nym& theNym,
        const Signature& theSignature) const -> bool;

    /** Uses authentication key instead of signing key. */
    auto VerifySigAuthent(
        const identity::Nym& theNym,
        const Signature& theSignature) const -> bool;
    auto VerifySignature(
        const crypto::key::Asymmetric& theKey,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool;
    auto GetContractPublicNym() const -> Nym_p;

protected:
    const api::Core& api_;

    /** Contract name as shown in the wallet. */
    OTString m_strName;

    /** Foldername for this contract (nyms, contracts, accounts, etc) */
    OTString m_strFoldername;

    /** Filename for this contract (usually an ID.) */
    OTString m_strFilename;

    /** Hash of the contract, including signatures. (the "raw file") */
    OTIdentifier m_ID;

    /** The Unsigned Clear Text (XML contents without signatures.) */
    OTStringXML m_xmlUnsigned;

    /** The complete raw file including signatures. */
    OTString m_strRawFile;

    /** The Hash algorithm used for the signature */
    crypto::HashType m_strSigHashType;

    /** CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM */
    OTString m_strContractType{String::Factory("CONTRACT")};

    /** The default behavior for a contract, though occasionally overridden, is
     * to contain its own public keys internally, located on standard XML tags.
     * So when we load a contract, we find its public key, and we verify its
     * signature with it. (It self-verifies!) I could be talking about an x509
     * as well, since people will need these to be revokable. The
     * Issuer/Server/etc URL will also be located within the contract, on a
     * standard tag, so by merely loading a contract, a wallet will know how to
     * connect to the relevant server, and the wallet will be able to encrypt
     * messages meant for that server to its public key without the normally
     * requisite key exchange. ==> THE TRADER HAS ASSURANCE THAT, IF HIS
     * OUT-MESSAGE IS ENCRYPTED, HE KNOWS THE MESSAGE CAN ONLY BE DECRYPTED BY
     * THE SAME PERSON WHO SIGNED THAT CONTRACT. */
    std::map<std::string, Nym_p> m_mapNyms;

    /** The PGP signatures at the bottom of the XML file. */
    listOfSignatures m_listSignatures;

    /** The version of this Contract file, in case the format changes in the
    future. */
    OTString m_strVersion{String::Factory("2.0")};

    // TODO: perhaps move these to a common ancestor for ServerContract and
    // OTUnitDefinition. Maybe call it OTHardContract (since it should never
    // change.)
    OTString m_strEntityShortName;
    OTString m_strEntityLongName;
    OTString m_strEntityEmail;

    /** The legal conditions, usually human-readable, on a contract. */
    String::Map m_mapConditions;

    /** The XML file is in m_xmlUnsigned-> Load it from there into members here.
     */
    auto LoadContractXML() -> bool;

    /** parses m_strRawFile into the various member variables. Separating these
     * into two steps allows us to load contracts from other sources besides
     * files. */
    auto ParseRawFile() -> bool;

    /** return -1 if error, 0 if nothing, and 1 if the node was processed. */
    virtual auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t;

    explicit Contract(const api::Core& core);
    explicit Contract(
        const api::Core& core,
        const String& name,
        const String& foldername,
        const String& filename,
        const String& strID);
    explicit Contract(const api::Core& core, const Identifier& theID);
    explicit Contract(const api::Core& core, const String& strID);

private:
    Contract() = delete;
};
}  // namespace opentxs
#endif
