#include "../pch.h"
#include "./headers/dllmain.h"

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
    return 0;
}