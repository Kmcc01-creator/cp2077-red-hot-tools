#include "RED4extProvider.hpp"

Support::RED4extProvider::RED4extProvider(RED4ext::v1::PluginHandle aPlugin, const RED4ext::v1::Sdk* aSdk) noexcept
    : m_plugin(aPlugin)
    , m_sdk(aSdk)
    , m_enableLogging(false)
    , m_enableHooking(false)
    , m_enableAddressLibrary(false)
{
}

void Support::RED4extProvider::OnInitialize()
{
    if (m_enableLogging)
    {
        LoggingDriver::SetDefault(*this);
    }

    if (m_enableHooking)
    {
        HookingDriver::SetDefault(*this);
    }

    if (m_enableAddressLibrary)
    {
        AddressResolver::SetDefault(*this);
    }
}

void Support::RED4extProvider::LogInfo(const std::string_view& aMessage)
{
    m_sdk->logger->Info(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogWarning(const std::string_view& aMessage)
{
    m_sdk->logger->Warn(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogError(const std::string_view& aMessage)
{
    m_sdk->logger->Error(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogDebug(const std::string_view& aMessage)
{
    m_sdk->logger->Debug(m_plugin, aMessage.data());
}

void Support::RED4extProvider::LogFlush()
{
}

// An address that did not resolve is 0 (see ResolveAddress); hooking it fails instead of patching
// whatever lies at 0.
bool Support::RED4extProvider::HookAttach(uintptr_t aAddress, void* aCallback)
{
    if (!aAddress)
        return false;

    return m_sdk->hooking->Attach(m_plugin, reinterpret_cast<void*>(aAddress), aCallback, nullptr);
}

bool Support::RED4extProvider::HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal)
{
    if (!aAddress)
        return false;

    return m_sdk->hooking->Attach(m_plugin, reinterpret_cast<void*>(aAddress), aCallback, aOriginal);
}

bool Support::RED4extProvider::HookDetach(uintptr_t aAddress)
{
    return m_sdk->hooking->Detach(m_plugin, reinterpret_cast<void*>(aAddress));
}

// The SDK's resolver shows an error and ends the process on a hash the game's address file lacks,
// as happens after a game update (five of RedHotTools' own hashes are gone on 2.5). RED4ext's own
// resolver returns 0 instead; the hook or call that needed the address then fails on its own.
uintptr_t Support::RED4extProvider::ResolveAddress(uint32_t aAddressID)
{
    using Resolve = uintptr_t (*)(uint32_t);
    static const auto resolve = reinterpret_cast<Resolve>(
        GetProcAddress(GetModuleHandleW(L"RED4ext.dll"), "RED4ext_ResolveAddress"));

    const auto address = resolve ? resolve(aAddressID) : 0;
    if (!address && m_sdk)
    {
        m_sdk->logger->ErrorF(m_plugin, "Address %u is not in this game's address file; its feature is off.",
                              aAddressID);
    }
    return address;
}
