#include "Lib.h"
#include "Winook.h"
#include "MessageSender.h"
#include "StreamLineReader.h"

#include <cinttypes>
#include <regex>
#include <string>

#define LOGWINOOKLIB 1
#if _DEBUG && LOGWINOOKLIB
#define LOGWINOOKLIBPATH TEXT("C:\\Temp\\WinookLibHookProc_")
#include "DebugHelper.h"
TimestampLogger Logger = TimestampLogger(LOGWINOOKLIBPATH + TimestampLogger::GetTimestampString(TRUE) + TEXT(".log"), TRUE);
#endif

asio::io_context io_context;
MessageSender messagesender(io_context);

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
#if _DEBUG
        LogDllMain(hinst, TEXT("DLL_PROCESS_ATTACH"));
#endif
        if (!Initialize(hinst))
        {
            return FALSE;
        }

        break;

    case DLL_THREAD_ATTACH:
#if _DEBUG
        LogDllMain(hinst, TEXT("DLL_THREAD_ATTACH"));
#endif
        break;

    case DLL_THREAD_DETACH:
#if _DEBUG
        LogDllMain(hinst, TEXT("DLL_THREAD_DETACH"));
#endif
        break;

    case DLL_PROCESS_DETACH:
#if _DEBUG
        LogDllMain(hinst, TEXT("DLL_PROCESS_DETACH"));
#endif
        break;
    }

    return TRUE;
}

BOOL Initialize(HINSTANCE hinst)
{
    // Look for initialization file stored in %TEMP%

    HMODULE module;
    if (!GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCTSTR)Initialize,
        &module))
    {
        return FALSE;
    }

    TCHAR dllfilepath[kPathBufferSize];
    GetModuleFileName(module, dllfilepath, kPathBufferSize);
#if _DEBUG && LOGWINOOKLIB
    Logger.WriteLine(TEXT("dllfilepath: ") + std::wstring(dllfilepath));
#endif
    int hooktype{};
    if (StrStrI(dllfilepath, kKeyboardHookLibName.c_str()) != NULL)
    {
        hooktype = WH_KEYBOARD;
    }
    else if (StrStrI(dllfilepath, kMouseHookLibName.c_str()) != NULL)
    {
        hooktype = WH_MOUSE;
    }
    else
    {
#if _DEBUG && LOGWINOOKLIB
        Logger.WriteLine(TEXT("Unsupported hook type."));
#endif
        return FALSE; // Unsupported hook type
    }

    const auto configfilepath = Winook::FindConfigFilePath(hooktype);
    if (configfilepath.empty())
    {
#if _DEBUG && LOGWINOOKLIB
        Logger.WriteLine(TEXT("configfilepath.empty()") + std::wstring(dllfilepath));
#endif
        return TRUE; // Assume out-of-context initialization
    }

#if _DEBUG && LOGWINOOKLIB
    Logger.WriteLine(TEXT("configfilepath: ") + configfilepath);
#endif

    StreamLineReader configfile(configfilepath);
    const auto port = std::stoi(configfile.ReadLine());
    configfile.Close();
    DeleteFile(configfilepath.c_str());
    messagesender.Connect(std::to_string(port));

    return TRUE;
}

#if _DEBUG
void LogDllMain(HINSTANCE hinst, std::wstring reason)
{
#if LOGWINOOKLIB
    std::wstringstream wss;
    wss << std::setw(16) << std::setfill(L'0') << std::hex << hinst;
    TCHAR procInfo[256];
    swprintf(procInfo, sizeof(procInfo), TEXT("Instance: %lx; Reason: %ls; ProcessId: %d; ThreadId: %d"), 
        PtrToLong(hinst), reason.c_str(), GetCurrentProcessId(), GetThreadId(GetCurrentThread()));
    Logger.WriteLine(procInfo);
#endif
}
#endif

LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
#if _DEBUG && LOGWINOOKLIB
    Logger.WriteLine(DebugHelper::FormatKeyboardHookMessage(code, wParam, lParam));
#endif
    if (code == HC_ACTION)
    {
        HookKeyboardMessage hkm;
        hkm.virtualKeyCode = (DWORD)wParam;
        hkm.flags = (DWORD)lParam;
        messagesender.SendMessage(&hkm, sizeof(HookKeyboardMessage));
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT CALLBACK MouseHookProc(int code, WPARAM wParam, LPARAM lParam) 
{
#if _DEBUG && LOGWINOOKLIB
    Logger.WriteLine(DebugHelper::FormatMouseHookMessage(code, wParam, lParam));
#endif
    if (code == HC_ACTION)
    {
        HookMouseMessage hmm{};
        hmm.messageCode = (DWORD)wParam;
        if (wParam == WM_MOUSEWHEEL)
        {
            auto pmhsx = (PMOUSEHOOKSTRUCTEX)lParam;
            hmm.pointX = pmhsx->pt.x;
            hmm.pointY = pmhsx->pt.y;
            hmm.hwnd = (DWORD)PtrToInt(pmhsx->hwnd);
            hmm.hitTestCode = (DWORD)pmhsx->wHitTestCode;
            hmm.zDelta = HIWORD(pmhsx->mouseData);
        }
        else
        {
            auto pmhs = (PMOUSEHOOKSTRUCT)lParam;
            hmm.pointX = pmhs->pt.x;
            hmm.pointY = pmhs->pt.y;
            hmm.hwnd = (DWORD)PtrToInt(pmhs->hwnd);
            hmm.hitTestCode = (DWORD)pmhs->wHitTestCode;
        }

        messagesender.SendMessage(&hmm, sizeof(HookMouseMessage));
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}