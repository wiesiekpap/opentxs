// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTBYLAW_HPP
#define OPENTXS_CORE_SCRIPT_OTBYLAW_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <string>

#include "opentxs/core/String.hpp"
#include "opentxs/core/script/OTVariable.hpp"

namespace opentxs
{
class OTClause;
class OTScript;
class OTScriptable;
class Tag;

using mapOfCallbacks = std::map<std::string, std::string>;
using mapOfClauses = std::map<std::string, OTClause*>;
using mapOfVariables = std::map<std::string, OTVariable*>;

// First is the name of some standard OT hook, like OnActivate, and Second is
// name of clause.
// It's a multimap because you might have 6 or 7 clauses that all trigger on the
// same hook.
//
using mapOfHooks = std::multimap<std::string, std::string>;

// A section of law, including its own clauses (scripts). A bylaw is kind of
// like an OT script "program", so it makes sense to be able to collect them,
// and to have them as discrete "packages".
//
class OTBylaw
{
    OTString m_strName;      // Name of this Bylaw.
    OTString m_strLanguage;  // Language that the scripts are written in, for
                             // this bylaw.

    mapOfVariables m_mapVariables;  // constant, persistant, and important
                                    // variables (strings and longs)
    mapOfClauses m_mapClauses;  // map of scripts associated with this bylaw.

    mapOfHooks m_mapHooks;  // multimap of server hooks associated with clauses.
                            // string / string
    mapOfCallbacks m_mapCallbacks;  // map of standard callbacks associated with
                                    // script clauses. string / string

    OTScriptable* m_pOwnerAgreement;  // This Bylaw is owned by an agreement
                                      // (OTScriptable-derived.)
    OTBylaw(const OTBylaw&) = delete;
    OTBylaw(OTBylaw&&) = delete;
    OTBylaw& operator=(const OTBylaw&) = delete;
    OTBylaw& operator=(OTBylaw&&) = delete;

public:
    OPENTXS_EXPORT const String& GetName() const { return m_strName; }
    OPENTXS_EXPORT const char* GetLanguage() const;
    OPENTXS_EXPORT bool AddVariable(OTVariable& theVariable);
    OPENTXS_EXPORT bool AddVariable(
        std::string str_Name,
        std::string str_Value,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    OPENTXS_EXPORT bool AddVariable(
        std::string str_Name,
        std::int32_t nValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    OPENTXS_EXPORT bool AddVariable(
        std::string str_Name,
        bool bValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    OPENTXS_EXPORT bool AddClause(OTClause& theClause);
    OPENTXS_EXPORT bool AddClause(const char* szName, const char* szCode);
    OPENTXS_EXPORT bool AddHook(
        std::string str_HookName,
        std::string str_ClauseName);  // name of hook such
                                      // as cron_process or
                                      // hook_activate, and
                                      // name of clause,
                                      // such as sectionA
                                      // (corresponding to
                                      // an actual script
                                      // in the clauses
                                      // map.)
    OPENTXS_EXPORT bool AddCallback(
        std::string str_CallbackName,
        std::string str_ClauseName);  // name of
                                      // callback such
                                      // as
    // callback_party_may_execute_clause,
    // and name of clause, such as
    // custom_party_may_execute_clause
    // (corresponding to an actual script
    // in the clauses map.)

    OPENTXS_EXPORT bool RemoveVariable(std::string str_Name);
    OPENTXS_EXPORT bool RemoveClause(std::string str_Name);
    OPENTXS_EXPORT bool RemoveHook(
        std::string str_Name,
        std::string str_ClauseName);
    OPENTXS_EXPORT bool RemoveCallback(std::string str_Name);

    OPENTXS_EXPORT bool UpdateClause(
        std::string str_Name,
        std::string str_Code);

    OPENTXS_EXPORT OTVariable* GetVariable(std::string str_Name);  // not a
                                                                   // reference,
                                                                   // so you can
                                                                   // pass in
                                                                   // char *.
                                                                   // Maybe
                                                                   // that's
                                                                   // bad? todo:
                                                                   // research
                                                                   // that.
    OPENTXS_EXPORT OTClause* GetClause(std::string str_Name) const;
    OPENTXS_EXPORT OTClause* GetCallback(std::string str_CallbackName);
    OPENTXS_EXPORT bool GetHooks(
        std::string str_HookName,
        mapOfClauses& theResults);  // Look up all clauses
                                    // matching a specific hook.
    OPENTXS_EXPORT std::int32_t GetVariableCount() const
    {
        return static_cast<std::int32_t>(m_mapVariables.size());
    }
    OPENTXS_EXPORT std::int32_t GetClauseCount() const
    {
        return static_cast<std::int32_t>(m_mapClauses.size());
    }
    OPENTXS_EXPORT std::int32_t GetCallbackCount() const
    {
        return static_cast<std::int32_t>(m_mapCallbacks.size());
    }
    OPENTXS_EXPORT std::int32_t GetHookCount() const
    {
        return static_cast<std::int32_t>(m_mapHooks.size());
    }
    OPENTXS_EXPORT OTVariable* GetVariableByIndex(std::int32_t nIndex);
    OPENTXS_EXPORT OTClause* GetClauseByIndex(std::int32_t nIndex);
    OPENTXS_EXPORT OTClause* GetCallbackByIndex(std::int32_t nIndex);
    OPENTXS_EXPORT OTClause* GetHookByIndex(std::int32_t nIndex);
    OPENTXS_EXPORT const std::string GetCallbackNameByIndex(
        std::int32_t nIndex);
    OPENTXS_EXPORT const std::string GetHookNameByIndex(std::int32_t nIndex);
    OPENTXS_EXPORT void RegisterVariablesForExecution(OTScript& theScript);
    OPENTXS_EXPORT bool IsDirty() const;  // So you can tell if any of the
                                          // persistent or important variables
                                          // have CHANGED since it was last set
                                          // clean.
    OPENTXS_EXPORT bool IsDirtyImportant() const;  // So you can tell if ONLY
                                                   // the IMPORTANT variables
                                                   // have CHANGED since it was
                                                   // last set clean.
    OPENTXS_EXPORT void SetAsClean();  // Sets the variables as clean, so you
                                       // can check
    // later and see if any have been changed (if it's
    // DIRTY again.)
    // This pointer isn't owned -- just stored for convenience.
    //
    OPENTXS_EXPORT OTScriptable* GetOwnerAgreement()
    {
        return m_pOwnerAgreement;
    }
    OPENTXS_EXPORT void SetOwnerAgreement(OTScriptable& theOwner)
    {
        m_pOwnerAgreement = &theOwner;
    }
    OPENTXS_EXPORT OTBylaw();
    OPENTXS_EXPORT OTBylaw(const char* szName, const char* szLanguage);
    virtual ~OTBylaw();

    OPENTXS_EXPORT bool Compare(OTBylaw& rhs);

    OPENTXS_EXPORT void Serialize(Tag& parent, bool bCalculatingID = false)
        const;
};

}  // namespace opentxs

#endif
