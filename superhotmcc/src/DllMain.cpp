#include <Windows.h>
#include <iostream>
#include <filesystem>
#include "utils/Console.hpp"
#include "haloce/Mod.hpp"
#include "MinHook.h"
#include "overlay/Overlay.hpp"

// MainThread
DWORD WINAPI MainThread(LPVOID _hModule) {
    HMODULE hModule = (HMODULE) _hModule;
    Console::alloc();
    Console::toggleConsole();

    MH_Initialize();
    Overlay::init();
    HaloCE::Mod::init();
    
    while (true) {
        if (GetAsyncKeyState(VK_F9) & 1)
            break;
        if (GetAsyncKeyState(VK_F8) & 1)
            Console::toggleConsole();
        HaloCE::Mod::modThreadUpdate();
        Sleep(1000 / 60);
    }
    
    HaloCE::Mod::free();
    Overlay::free();
    MH_Uninitialize();

    // Allow time for hooks to exit.
    Sleep(200);

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