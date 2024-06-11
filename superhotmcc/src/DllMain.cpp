#include <Windows.h>
#include "DllMain.hpp"
#include <iostream>
#include <filesystem>
#include "utils/Console.hpp"
#include "haloce/Mod.hpp"
#include "MinHook.h"
#include "overlay/Overlay.hpp"
#include "utils/UnloadLock.hpp"

namespace ModHost {
    bool bReinit = false;
    bool bExit = false;
    void reinitialize() {
        std::cout << "Reinitializing..." << std::endl;
        bReinit = true;
    }
    void exit() {
        std::cout << "Exiting..." << std::endl;
        bExit = true;
    }
}

// MainThread
DWORD WINAPI MainThread(LPVOID _hModule) {
    HMODULE hModule = (HMODULE) _hModule;
    Console::alloc();
    Console::toggleConsole();

    do {
        ModHost::bReinit = false;
        
        MH_Initialize();
        Overlay::init();
        
        while (!ModHost::bExit && !ModHost::bReinit) {
            if (GetAsyncKeyState(VK_F8) & 1)
                Console::toggleConsole();
            if (GetAsyncKeyState(VK_F9) & 1) {
                ModHost::exit();
                break;
            }
            if (GetAsyncKeyState(VK_F10) & 1) {
                ModHost::reinitialize();
                break;
            }
            HaloCE::Mod::modThreadUpdate();
            Sleep(1000 / 60);
        }
        
        waitForSafeUnload();
        HaloCE::Mod::free();
        Overlay::free();
        MH_Uninitialize();
        
    } while (ModHost::bReinit);


    waitForSafeUnload();
    Sleep(200); // Extra time for hooks to exit.

    Console::free();

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

// Dll Main
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, MainThread, hModule, 0, 0);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}