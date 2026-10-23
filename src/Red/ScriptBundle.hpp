#pragma once

#include <cstdint>
#include <RED4ext/CName.hpp>
#include <RED4ext/Containers/DynArray.hpp>
#include <RED4ext/HashMap.hpp>
#include <RED4ext/Memory/Allocators.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>

// Game 2.5 layouts, recovered from the exe's constructors, destructors, serializers and binder (the
// verifier's docs/hot-reload.md, "Script structures on 2.5"). The definition base grew from 0x18 to 0x40,
// so most fields sit 0x28 past 2.31's (given as "2.31: XX"); the rest are noted where they moved further.
// Fields named unkXX are placed but not understood; several serve 2.5's own script mods.

namespace Red
{
enum class EScriptDefinition : uint8_t
{
    Type = 0,
    Class,
    Constant, // Values for Enums and Bitfields
    Enum,
    Bitfield, // Game 2.5 builds these with the Enum constructor, whose GetType returns Enum
    Function,
    Parameter,
    LocalVar,
    Property,
    SourceFile,
    Unk10, // Game 2.5: a new 0x50-byte kind; none in the game's own bundle
};

enum class EScriptVisibility : uint8_t
{
    Public = 0,
    Protected,
    Private,
};

struct ScriptDefinition
{
    virtual Memory::IAllocator* GetAllocator() = 0; // 00
    virtual ~ScriptDefinition() = 0;                // 08
    virtual EScriptDefinition GetType() = 0;        // 10
    virtual EScriptVisibility GetVisibility() = 0;  // 18
    virtual ScriptDefinition* GetParent() = 0;      // 20 - A function's class; null for a class
    virtual ScriptDefinition* GetBase() = 0;        // 28 - A class's base class; null for a function
    virtual bool IsNative() = 0;                    // 30

    CName name;          // 08
    uint32_t unk10;      // 10
    uint8_t unk14;       // 14
    uint64_t unk18;      // 18
    uint8_t flags;       // 20 - Game 2.5: bit 0 for a definition from another bundle; native resolution skips it
    CName unk28;         // 28
    DynArray<void*> unk30; // 30
};
RED4EXT_ASSERT_SIZE(ScriptDefinition, 0x40);

struct ScriptFileInfo : ScriptDefinition
{
    uint32_t crc;   // 40 (2.31: 18)
    uint32_t index; // 44 (2.31: 1C)
    uint32_t hash;  // 48 (2.31: 20) - Murmur3 of path
    CString path;   // 50 (2.31: 28)
};
RED4EXT_ASSERT_SIZE(ScriptFileInfo, 0x70);

enum class EScriptType : uint8_t
{
    Primitive = 0,
    Class, // Class/Enum/Bitfield
    StrongHandle,
    WeakHandle,
    DynArray,
    StaticArray,
    ScriptRef,
};

struct ScriptType : ScriptDefinition
{
    CBaseRTTIType* rttiType;     // 40 (2.31: 18)
    ScriptDefinition* innerType; // 48 (2.31: 20)
    int32_t arraySize;           // 50 (2.31: 28)
    EScriptType metaType;        // 54 (2.31: 2C)
};
RED4EXT_ASSERT_SIZE(ScriptType, 0x58);
RED4EXT_ASSERT_OFFSET(ScriptType, metaType, 0x54);

struct ScriptProperty : ScriptDefinition
{
    struct Flags
    {
        uint16_t isNative : 1;       // 00
        uint16_t b01 : 1;            // 01
        uint16_t b02 : 1;            // 02
        uint16_t isConst : 1;        // 03
        uint16_t b04 : 1;            // 04
        uint16_t b05 : 1;            // 05
        uint16_t isPersistent : 1;   // 06
        uint16_t b07 : 1;            // 07
        uint16_t b08 : 1;            // 08
        uint16_t b09 : 1;            // 09
        uint16_t b0A : 1;            // 0A
        uint16_t isAddedToClass : 1; // 0B - Game 2.5: a mod's field, added to the class named addedTo
    };

    CProperty* rttiProp;                // 40 (2.31: 18)
    ScriptDefinition* parent;           // 48 (2.31: 20)
    Flags flags;                        // 50 (2.31: 28)
    Map<CString, CString> defaults;     // 58 (2.31: 30)
    EScriptVisibility visibility;       // 80 (2.31: 58)
    HashMap<CName, CString> attributes; // 88 (2.31: 60)
    ScriptType* type;                   // B8 (2.31: 90)
    CName addedTo;                      // C0 - Game 2.5
};
RED4EXT_ASSERT_SIZE(ScriptProperty, 0xC8);
RED4EXT_ASSERT_OFFSET(ScriptProperty, visibility, 0x80);
RED4EXT_ASSERT_OFFSET(ScriptProperty, type, 0xB8);

struct ScriptClass;
struct ScriptFunction;

struct ScriptParameter : ScriptDefinition
{
    struct Flags
    {
        uint8_t isOptional : 1;     // 00
        uint8_t isOut : 1;          // 01
        uint8_t isShortCircuit : 1; // 02 - Got from redscript
        uint8_t isConst : 1;        // 03
    };

    CProperty* rttiProp;      // 40 (2.31: 18)
    ScriptFunction* function; // 48 (2.31: 20)
    ScriptType* type;         // 50 (2.31: 28)
    Flags flags;              // 58 (2.31: 30)
};
RED4EXT_ASSERT_SIZE(ScriptParameter, 0x60);

struct ScriptLocalVar : ScriptDefinition
{
    CProperty* rttiProp;      // 40 (2.31: 18)
    ScriptFunction* function; // 48 (2.31: 20)
    ScriptType* type;         // 50 (2.31: 28)
    uint8_t unk58;            // 58 (2.31: 30)
};
RED4EXT_ASSERT_SIZE(ScriptLocalVar, 0x60);

struct ScriptFunction : ScriptDefinition
{
    struct Flags
    {
        uint32_t isStatic : 1;     // 00
        uint32_t isExec : 1;       // 01
        uint32_t isTimer : 1;      // 02
        uint32_t isFinal : 1;      // 03
        uint32_t isNative : 1;     // 04
        uint32_t b05 : 1;          // 05
        uint32_t b06 : 1;          // 06
        uint32_t isEvent : 1;      // 07
        uint32_t b08 : 1;          // 08
        uint32_t b09 : 1;          // 09
        uint32_t b10 : 1;          // 0A
        uint32_t b11 : 1;          // 0B
        uint32_t b12 : 1;          // 0C
        uint32_t b13 : 1;          // 0D
        uint32_t b14 : 1;          // 0E
        uint32_t isConst : 1;      // 0F
        uint32_t isThreadsafe : 1; // 10
        uint32_t isScripted : 1;   // 11 - Seems pointless, for all functions isScripted != isNative
        uint32_t b18_23 : 6;       // 12
        // Game 2.5, the fourth byte: set from the file's flag bits 28, 29 and 31, and by the binder.
        uint32_t isPatch : 1;      // 18 - Patches patchTarget, as patchType says
        uint32_t b25 : 1;          // 19 - With it, unkE0 and unkF0 are read
        uint32_t hasOpcodes : 1;   // 1A - LoadOpcodes translates only functions with it; set on construction
        uint32_t b27 : 1;          // 1B
        uint32_t patchType : 4;    // 1C
    };

    CBaseFunction* rttiFunc;             // 40 (2.31: 18)
    ScriptClass* parent;                 // 48 (2.31: 20)
    CName shortName;                     // 50 (2.31: 28)
    DynArray<ScriptParameter*> params;   // 58 (2.31: 30)
    DynArray<ScriptLocalVar*> localVars; // 68 (2.31: 40)
    HashMap<CName, CString> attributes;  // 78 (2.31: 50)
    CName unkA8;                         // A8 - Game 2.5: a class name, for patch type 3
    CName unkB0;                         // B0 - Game 2.5
    ScriptType* returnType;              // B8 (2.31: 80)
    uint8_t unkC0;                       // C0 (2.31: 88)
    ScriptFunction* patchTarget;         // C8 - Game 2.5
    ScriptFunction* unkD0;               // D0 - Game 2.5
    ScriptFunction* overridden;          // D8 (2.31: 90)
    DynArray<void*> unkE0;               // E0 - Game 2.5
    ScriptFunction* unkF0;               // F0 - Game 2.5: LoadOpcodes translates it in this one's place
    Flags flags;                         // F8 (2.31: 98)
    DynArray<void*> unk100;              // 100 - Game 2.5
    DynArray<void*> unk110;              // 110 - Game 2.5
    CName operatorName;                  // 120 (2.31: A0)
    uint8_t castValue;                   // 128 (2.31: A8)
    EScriptVisibility visibility;        // 129 (2.31: A9)
    DynArray<uint8_t> code;              // 130 (2.31: B0)
    ScriptFileInfo* sourceFile;          // 140 (2.31: C0)
    uint32_t sourceLine;                 // 148 (2.31: C8)
    uint8_t unk14C;                      // 14C - Game 2.5
};
RED4EXT_ASSERT_SIZE(ScriptFunction, 0x150);
RED4EXT_ASSERT_OFFSET(ScriptFunction, returnType, 0xB8);
RED4EXT_ASSERT_OFFSET(ScriptFunction, overridden, 0xD8);
RED4EXT_ASSERT_OFFSET(ScriptFunction, flags, 0xF8);
RED4EXT_ASSERT_OFFSET(ScriptFunction, visibility, 0x129);
RED4EXT_ASSERT_OFFSET(ScriptFunction, code, 0x130);
RED4EXT_ASSERT_OFFSET(ScriptFunction, sourceLine, 0x148);

struct ScriptClass : ScriptDefinition
{
    struct Flags
    {
        uint8_t isNative : 1;     // 00
        uint8_t isImportOnly : 1; // 01
        uint8_t isAbstract : 1;   // 02
        uint8_t isFinal : 1;      // 03
        uint8_t isStruct : 1;     // 04
    };

    CClass* rttiClass;                    // 40 (2.31: 18)
    ScriptClass* parent;                  // 48 (2.31: 20)
    DynArray<ScriptProperty*> properties; // 50 (2.31: 28)
    DynArray<ScriptProperty*> overrides;  // 60 (2.31: 38)
    DynArray<void*> unk70;                // 70 - Game 2.5; elements not owned
    DynArray<void*> unk80;                // 80 - Game 2.5; elements not owned
    DynArray<ScriptFunction*> functions;  // 90 (2.31: 48)
    HashMap<CName, CString> attributes;   // A0 (2.31: 58)
    EScriptVisibility visibility;         // D0 (2.31: 88)
    Flags flags;                          // D1 (2.31: 89)
};
RED4EXT_ASSERT_SIZE(ScriptClass, 0xD8);
RED4EXT_ASSERT_OFFSET(ScriptClass, functions, 0x90);
RED4EXT_ASSERT_OFFSET(ScriptClass, visibility, 0xD0);

struct ScriptConstant : ScriptDefinition
{
    int64_t rttiValue;        // 40 (2.31: 18) - Used only for natives
    ScriptDefinition* parent; // 48 (2.31: 20) - Enum or Bitfield
    int64_t value;            // 50 (2.31: 28)
    bool isNative;            // 58 (2.31: 30)
};
RED4EXT_ASSERT_SIZE(ScriptConstant, 0x60);

struct ScriptEnum : ScriptDefinition
{
    CEnum* rttiEnum;                          // 40 (2.31: 18)
    DynArray<ScriptConstant*> constants;      // 48 (2.31: 20)
    DynArray<ScriptConstant*> extraConstants; // 58 - Game 2.5: CreateEnum adds them as well
    uint8_t size;                             // 68 (2.31: 30)
    bool isNative;                            // 69 (2.31: 31)
    EScriptVisibility visibility;             // 6A (2.31: 32)
};
RED4EXT_ASSERT_SIZE(ScriptEnum, 0x70);
RED4EXT_ASSERT_OFFSET(ScriptEnum, isNative, 0x69);

// Game 2.5 builds bitfields with the Enum constructor, so they share its layout.
struct ScriptBitfield : ScriptDefinition
{
    CBitfield* rttiBitfield;                  // 40 (2.31: 18)
    DynArray<ScriptConstant*> constants;      // 48 (2.31: 20)
    DynArray<ScriptConstant*> extraConstants; // 58 - Game 2.5
    uint8_t size;                             // 68 (2.31: 30)
    bool isNative;                            // 69 (2.31: 31)
    EScriptVisibility visibility;             // 6A (2.31: 32)
};
RED4EXT_ASSERT_SIZE(ScriptBitfield, 0x70);

struct ScriptBundle
{
    ScriptBundle();
    ~ScriptBundle();

    bool Read(const CString& aPath);
    [[nodiscard]] DynArray<ScriptDefinition*> Collect(bool aDeep = false) const;

    uint8_t unk00;                                       // 00 - Game 2.5
    HashMap<CName, ScriptDefinition*> definitionsByName; // 08 (2.31: 00)
    HashMap<CName, ScriptFunction*> functionsByName;     // 38 (2.31: 30)
    HashMap<CName, ScriptType*> typesByName;             // 68 (2.31: 60)
    DynArray<ScriptFileInfo*> files;                     // 98 (2.31: 90)
    DynArray<ScriptFunction*> globals;                   // A8 (2.31: A0)
    DynArray<ScriptEnum*> enums;                         // B8 (2.31: B0)
    DynArray<ScriptBitfield*> bitfields;                 // C8 (2.31: C0)
    DynArray<ScriptClass*> classes;                      // D8 (2.31: D0)
    DynArray<ScriptType*> types;                         // E8 (2.31: E0)
    DynArray<CString> strings;                           // F8 (2.31: F0)
    HashMap<CName, void*> unk108;                        // 108 (2.31: 100)
    SharedSpinLock typesLock;                            // 138 (2.31: 130)
    CRITICAL_SECTION unk140;                             // 140 (2.31: 138)
    // Game 2.5, for its own script mods:
    DynArray<CString> unk168;                            // 168
    uint32_t unk178;                                     // 178
    ScriptBundle* outer;                                 // 180 - Looked up after this bundle's own definitions
    DynArray<void*> unk188;                              // 188
    DynArray<void*> unk198;                              // 198
    CString unk1A8;                                      // 1A8
    uint32_t unk1C8;                                     // 1C8
    DynArray<ScriptProperty*> addedProperties;           // 1D0 - Made while reading, from isAddedToClass fields
    DynArray<void*> unk1E0;                              // 1E0
    uint8_t unk1F0;                                      // 1F0
    uint8_t unk1F1;                                      // 1F1
};
RED4EXT_ASSERT_SIZE(ScriptBundle, 0x1F8);
RED4EXT_ASSERT_OFFSET(ScriptBundle, globals, 0xA8);
RED4EXT_ASSERT_OFFSET(ScriptBundle, classes, 0xD8);
RED4EXT_ASSERT_OFFSET(ScriptBundle, strings, 0xF8);
RED4EXT_ASSERT_OFFSET(ScriptBundle, unk140, 0x140);
RED4EXT_ASSERT_OFFSET(ScriptBundle, addedProperties, 0x1D0);
}
