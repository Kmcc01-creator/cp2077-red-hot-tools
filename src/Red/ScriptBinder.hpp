#pragma once

#include "ScriptBundle.hpp"
#include "ScriptReport.hpp"

#include <functional>
#include <RED4ext/Containers/DynArray.hpp>
#include <RED4ext/RTTISystem.hpp>

namespace Red
{
struct ScriptFile
{
    uint32_t hash;
    uint32_t crc;
    CString path;
};

using FileResolver = std::function<ScriptFile*(uint32_t)>;

struct ScriptBinder
{
    ScriptBinder(CRTTISystem* aRtti, const FileResolver& aFileResolver);

    // bool Bind(ScriptBundle& aData, ScriptReport& aReport);
    bool ResolveNatives(DynArray<ScriptDefinition*>& aDefinitions, ScriptReport& aReport);
    void ResolveTypes(DynArray<ScriptDefinition*>& aDefinitions);
    bool CreateClass(ScriptClass* aClass, ScriptReport& aReport);
    bool CreateEnum(ScriptEnum* aEnum, ScriptReport& aReport);
    bool CreateBitfield(ScriptBitfield* aBitfield, ScriptReport& aReport);
    bool CreateProperties(ScriptClass* aClass, ScriptReport& aReport);
    bool CreateFunction(ScriptFunction* aFunc, ScriptReport& aReport);
    void TranslateBytecode(DynArray<ScriptDefinition*>& aDefinitions);

    // Game 2.5: destroys the value holder an RTTI class keeps at 0xE0, if any. The game's binder does it
    // for every class in the bundle before it creates types, and ClearScriptData does it again.
    static void ResetClass(CClass* aClass);

    CRTTISystem* rtti;
    FileResolver fileResolver;
};
}
