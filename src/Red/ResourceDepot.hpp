#pragma once

namespace Raw::ResourceDepot
{
constexpr auto LoadArchives = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_LoadArchives,
    /* type = */ void (*)(Red::ResourceDepot* aDepot,
                          Red::ArchiveGroup& aGroup,
                          const Red::DynArray<Red::CString>& aArchivePaths,
                          Red::DynArray<Red::ResourcePath>& aLoadedResourcePaths,
                          uint32_t aMemoryResident,
                          uint8_t aSource)>{};
// aMemoryResident is a dword: 2.31's startup loader passes 1 for its "memoryresident_" group.
// aSource (named here, meaning unknown) is 2 from the startup loader's base groups and 0 from the
// mod archive loader, in 2.31 and 2.5 alike; the declaration lacked it before, so the byte was
// whatever lay on the stack.

constexpr auto DestructArchive = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_DestructArchive,
    /* type = */ void (*)(Red::Archive* aArchive)>{};

constexpr auto DestructArchives = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_DestructArchives,
    /* type = */ void (*)(Red::Archive aArchives[], uint32_t aCount)>{};

constexpr auto RequestResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_RequestResource,
    /* type = */ uintptr_t* (*)(Red::ResourceDepot* aDepot,
                                const uintptr_t* aOutResourceHandle,
                                Red::ResourcePath aPath,
                                const int32_t* aArchiveHandle)>{};
}

namespace Raw::ResourceReference
{
constexpr auto EnsureLoaded = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceReference_EnsureLoaded,
    /* type = */ void (*)(Red::ResourceReference<>* aReference)>{};
}
