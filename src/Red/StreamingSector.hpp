#pragma once

#include "Red/Common.hpp"

namespace Red
{
struct __declspec(align(0x10)) CompiledNodeInstanceSetupInfo
{
    Transform transform;               // 00
    Vector3 scale;                     // 20
    Vector3 secondaryRefPointPosition; // 2C
    Vector3 streamingRefPoint;         // 38
    Vector3 unk44;                     // 44
    worldNode* node;                   // 50
    uint64_t globalNodeID;             // 58
    uint64_t proxyNodeID;              // 60
    ResourcePath unk68;                // 68
    // Game 2.5: the engine reads the node index as the low 19 bits of the dword at 0x70, where 2.31
    // had secondaryRefPointDistance; it had a uint16 at 0x78. 0x74 to 0x7F are unchecked on 2.5,
    // including unk7A's interior flag (0x40), as is proxyNodeID at 0x60.
    uint32_t nodeIndex : 19;           // 70
    uint32_t unk70 : 13;               // 70
    float streamingDistance;           // 74
    uint16_t unk78;                    // 78
    uint16_t unk7A;                    // 7A
    uint16_t unk7C;                    // 7C
    uint16_t unk7E;                    // 7E
    uint64_t unk80;                    // 80
    uint64_t unk88;                    // 88
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfo, 0x90);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, node, 0x50);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, globalNodeID, 0x58);

// Game 2.5: 8 bytes larger than a DataBuffer; its constructor sets a dword at 0x28 to 0x44.
struct CompiledNodeInstanceSetupInfoBuffer : DataBuffer
{
    [[nodiscard]] inline CompiledNodeInstanceSetupInfo* begin() const
    {
        return reinterpret_cast<CompiledNodeInstanceSetupInfo*>(buffer.data);
    }

    [[nodiscard]] inline CompiledNodeInstanceSetupInfo* end() const
    {
        return reinterpret_cast<CompiledNodeInstanceSetupInfo*>(reinterpret_cast<uintptr_t>(buffer.data) +
                                                                buffer.size);
    }

    uint32_t unk28; // 28
    uint32_t unk2C; // 2C
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfoBuffer, 0x30);

// Game 2.5, from its constructor and destructor in the exe: arrays of 16-, 8- and 4-byte elements
// at 0x30, 0x40 and 0x50, and a dword set to -1 at 0x60. The engine reads nodes at 0x30.
struct StreamingSectorNodeBuffer
{
    CompiledNodeInstanceSetupInfoBuffer nodeSetups; // 00
    DynArray<Handle<worldNode>> nodes;              // 30
    DynArray<NodeRef> nodeRefs;                     // 40
    DynArray<uint32_t> unk50;                       // 50
    int32_t unk60;                                  // 60
};
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodes, 0x30);
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodeRefs, 0x40);
}

namespace Raw::StreamingSector
{
using NodeBuffer = Core::OffsetPtr<0x40, Red::StreamingSectorNodeBuffer>;

constexpr auto Destruct = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingSector_dtor,
    /* type = */ void (*)(Red::worldStreamingSector* aSector)>();

constexpr auto PostLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingSector_OnReady,
    /* type = */ void (*)(Red::worldStreamingSector* aSector, uint64_t a2)>();
}
