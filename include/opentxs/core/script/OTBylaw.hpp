// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTBYLAW_HPP
#define OPENTXS_CORE_SCRIPT_OTBYLAW_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
class OPENTXS_EXPORT OTBylaw
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
    const String& GetName() const { return m_strName; }
    const char* GetLanguage() const;
    bool AddVariable(OTVariable& theVariable);
    bool AddVariable(
        std::string str_Name,
        std::string str_Value,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    bool AddVariable(
        std::string str_Name,
        std::int32_t nValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    bool AddVariable(
        std::string str_Name,
        bool bValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    bool AddClause(OTClause& theClause);
    bool AddClause(const char* szName, const char* szCode);
    bool AddHook(
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
    bool AddCallback(
        std::string str_CallbackName,
        std::string str_ClauseName);  // name of
                                      // callback such
                                      // as
    // callback_party_may_execute_clause,
    // and name of clause, such as
    // custom_party_may_execute_clause
    // (corresponding to an actual script
    // in the clauses map.)

    bool RemoveVariable(std::string str_Name);
    bool RemoveClause(std::string str_Name);
    bool RemoveHook(std::string str_Name, std::string str_ClauseName);
    bool RemoveCallback(std::string str_Name);

    bool UpdateClause(std::string str_Name, std::string str_Code);

    OTVariable* GetVariable(std::string str_Name);  // not a
                                                    // reference,
                                                    // so you can
                                                    // pass in
                                                    // char *.
                                                    // Maybe
                                                    // that's
                                                    // bad? todo:
                                                    // research
                                                    // that.
    OTClause* GetClause(std::string str_Name) const;
    OTClause* GetCallback(std::string str_CallbackName);
    bool GetHooks(
        std::string str_HookName,
        mapOfClauses& theResults);  // Look up all clauses
                                    // matching a specific hook.
    std::int32_t GetVariableCount() const
    {
        return static_cast<std::int32_t>(m_mapVariables.size());
    }
    std::int32_t GetClauseCount() const
    {
        return static_cast<std::int32_t>(m_mapClauses.size());
    }
    std::int32_t GetCallbackCount() const
    {
        return static_cast<std::int32_t>(m_mapCallbacks.size());
    }
    std::int32_t GetHookCount() const
    {
        return static_cast<std::int32_t>(m_mapHooks.size());
    }
    OTVariable* GetVariableByIndex(std::int32_t nIndex);
    OTClause* GetClauseByIndex(std::int32_t nIndex);
    OTClause* GetCallbackByIndex(std::int32_t nIndex);
    OTClause* GetHookByIndex(std::int32_t nIndex);
    const std::string GetCallbackNameByIndex(std::int32_t nIndex);
    const std::string GetHookNameByIndex(std::int32_t nIndex);
    void RegisterVariablesForExecution(OTScript& theScript);
    bool IsDirty() const;           // So you can tell if any of the
                                    // persistent or important variables
                                    // have CHANGED since it was last set
                                    // clean.
    bool IsDirtyImportant() const;  // So you can tell if ONLY
                                    // the IMPORTANT variables
                                    // have CHANGED since it was
                                    // last set clean.
    void SetAsClean();              // Sets the variables as clean, so you
                                    // can check
    // later and see if any have been changed (if it's
    // DIRTY again.)
    // This pointer isn't owned -- just stored for convenience.
    //
    OTScriptable* GetOwnerAgreement() { return m_pOwnerAgreement; }
    void SetOwnerAgreement(OTScriptable& theOwner)
    {
        m_pOwnerAgreement = &theOwner;
    }
    OTBylaw();
    OTBylaw(const char* szName, const char* szLanguage);
    virtual ~OTBylaw();

    bool Compare(OTBylaw& rhs);

    void Serialize(Tag& parent, bool bCalculatingID = false) const;
};

}  // namespace opentxs

#endif
