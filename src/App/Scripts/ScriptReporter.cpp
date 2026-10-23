#include "ScriptReporter.hpp"
#include "Red/EngineConfig.hpp"
#include "Red/GameEngine.hpp"

#include <filesystem>

namespace
{
// Where RED4ext writes the modded blob: beside the unmodified one, as <name>.redscripts.modded.
std::filesystem::path ModdedBlobPath(const Red::CString& aBlobPath)
{
    return std::filesystem::path(aBlobPath.c_str()).replace_extension("redscripts.modded");
}
}

App::ScriptReporter::ScriptReporter(bool aCompatMode)
    : m_compatMode(aCompatMode)
{
}

void App::ScriptReporter::ShowErrorBox(const char* aCaption, const char* aMessage)
{
    auto engine = Red::CGameEngine::Get();
    auto hWnd = engine->unkD0 ? reinterpret_cast<HWND>(engine->unkD0->hWnd) : nullptr;

    MessageBoxA(hWnd, aMessage, aCaption, MB_SYSTEMMODAL | MB_ICONERROR);
}

void App::ScriptReporter::ShowErrorBox(const char* aCaption, const std::string& aMessage)
{
    ShowErrorBox(aCaption, aMessage.c_str());
}

void App::ScriptReporter::ShowErrorBox(const char* aCaption, const Red::CString& aMessage)
{
    ShowErrorBox(aCaption, aMessage.c_str());
}

void App::ScriptReporter::ShowErrorBox(const char* aCaption, const Red::DynArray<Red::CString>& aMessages)
{
    std::string message;

    bool eol = false;
    for (const auto& entry : aMessages)
    {
        if (eol)
        {
            message.append("\n");
        }

        message.append(entry.c_str());
        eol = true;
    }

    ShowErrorBox(aCaption, message.c_str());
}

void App::ScriptReporter::OnBootstrap()
{
    Hook<Raw::CBaseEngine::LoadScripts>(&ScriptReporter::OnLoadScripts);
}

bool App::ScriptReporter::OnLoadScripts(Red::CBaseEngine& aEngine, Red::CString& aPath,
                                        uint64_t aTimestamp, uint64_t a4)
{
    // This is a fallback mode which never works for the release game
    if (!aTimestamp)
        return false;

    const auto compilationEnabled = Red::EngineConfig::Get()->GetValue<bool>("Scripts", "EnableCompilation");
    // Game 2.5: RED4ext no longer sets the engine's blob path (it redirects loads of the unmodified blob to the modded
    // one instead), so an empty path no longer means a failed compilation. It still does where RED4ext sets it.
    const auto blobPathUnset = aEngine.scriptsBlobPath.Length() == 0 && !std::filesystem::exists(ModdedBlobPath(aPath));
    if (aEngine.scriptsCompilationErrors.Length() > 0 || (compilationEnabled && blobPathUnset))
    {
        ExitProcess(0xDEAD);
        return false;
    }

    // When enabled the errors will be added to the CBaseEngine::scriptsValidationErrors
    aEngine.scriptsSilentValidation = true;

    bool success = Raw::CBaseEngine::LoadScripts(aEngine, aPath, aTimestamp, a4);

    if (!success && !aEngine.scriptsValidationErrors.IsEmpty())
    {
        ShowErrorBox("Validation error", aEngine.scriptsValidationErrors);
        ExitProcess(0xDEAD);
        return false;
    }

    return success;
}
