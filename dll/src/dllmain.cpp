#include "../pch.h"
#include "./headers/dllmain.h"
#include "./headers/DX11Hook.h"
#include "./headers/Hook.h"
#include "./headers/AllocationUtils.h"
#include "./utils/headers/keypressed.h"

HMODULE hmSuperHotHack;
HMODULE hmHalo1;
UINT_PTR halo1Base;

BOOL APIENTRY DllMain( 
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            hmSuperHotHack = hModule;
            hmHalo1 = GetModuleHandleA("halo1.dll");
            halo1Base = (UINT_PTR) hmHalo1;
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

    std::cout << "MCC-SUPERHOT Mod Loaded\n\n";

    std::cout << "halo1.dll located at: ";
    std::cout << std::uppercase << std::hex << hmHalo1;
    std::cout << "\n\n";

    DX11Hook::init();
    testHook();

    if (!err) {
        while ( !GetAsyncKeyState(VK_F9) ) {

            // if ( keypressed(VK_F6) )
            //     AllocationUtils::test_virtualAllocNear(0x00007FFBEA568000U, 1000);
            
            Sleep(10);

        }
    }

    std::cout << "Exiting..." << std::endl;
    
    Hook::cleanupHooks();
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

    FreeLibraryAndExitThread(hmSuperHotHack, 0);
    
}

void onDamageEntity() {
    std::cout << "Printing from hook." << std::endl;
}

void onDamageEntity2() {
    std::cout << "Printing from hook 2." << std::endl;
}

void testHook() {

    Hook::addJumpHook(
        "Test hook 1",
        halo1Base+0xC1909CU, 8,
        (UINT_PTR) onDamageEntity,
        HK_PUSH_STATE
    );

    Hook::addJumpHook(
        "Test hook 2",
        halo1Base+0xC18E60U, 7,
        (UINT_PTR) onDamageEntity2,
        HK_PUSH_STATE 
    );

}
