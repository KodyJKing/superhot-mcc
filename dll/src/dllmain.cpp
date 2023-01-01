#include "../pch.h"
#include "./headers/dllmain.h"
#include "./headers/DX11Hook.h"
#include "./headers/Hook.h"

HMODULE thisHModule;

BOOL APIENTRY DllMain( 
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            thisHModule = hModule;
            CreateThread(0, 0, mainThread, 0, 0, 0);
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

DWORD __stdcall mainThread(LPVOID lpParameter) {

    const bool useConsole = true;
    const bool useStdin = false;
    const bool pressKeyToExit = false;

    errno_t err = 0;
    FILE *pFile_stdout, *pFile_stderr, *pFile_stdin;
    if (useConsole) {
        AllocConsole();
        err = freopen_s(&pFile_stdout, "CONOUT$", "w", stdout);
        err |= freopen_s(&pFile_stderr, "CONOUT$", "w", stderr);
        if (useStdin)
            err |= freopen_s(&pFile_stdin, "CONIN$", "r", stdin);
    }

    std::cout << "MCC-SUPERHOT Mod Loaded" << std::endl;

    DX11Hook::init();
    testHook();

    if (!err) {
        while (TRUE) {
            if (GetAsyncKeyState(VK_F9))
                break;
            Sleep(16);
        }
    }

    std::cout << "Exiting..." << std::endl;
    
    Hook::removeAllJumpHookRecords();
    Sleep(500);

    if (useConsole) {
        if (useStdin && pressKeyToExit) {
            std::cout << std::endl << "Press anything to continue" << std::endl;
            getchar();
        }

        if (pFile_stdout) fclose(pFile_stdout);
        if (pFile_stderr) fclose(pFile_stderr);
        if (useStdin && pFile_stdin) fclose(pFile_stdin);
        FreeConsole();
    }

    FreeLibraryAndExitThread(thisHModule, 0);
    
}

void onDamageEntity() {
    std::cout << "Printing from hook." << std::endl;
}

void testHook() {

    Hook::addJumpHook(
        "Insta Kill",
        0x7FFBEF60909CU,
        8,
        (UINT_PTR) onDamageEntity,
        HK_PUSH_STATE
    );

}
