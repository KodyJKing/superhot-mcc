#include "headers/HaloMCC.h"
#include "headers/Hook.h"

namespace HaloMCC {

    static const char* moduleName = "mcc-win64-shipping.exe";

    HWND getWindow() {
        static HWND window = NULL;
        if ( !window )
            window = FindWindowA( MCC_WINDOW_CLASS_NAME, MCC_WINDOW_NAME );
        return window;
    }

    Vec2 getWindowSize() {
        HWND hwnd = getWindow();
        RECT rect;
        GetClientRect( hwnd, &rect );
        return {
            (float) ( rect.right - rect.left ),
            (float) ( rect.bottom - rect.top )
        };
    }

    bool isInForeground() {
        return getWindow() == GetForegroundWindow();
    }

    uint64_t getModuleBase() {
        return (uint64_t) GetModuleHandleA( moduleName );
    }

    IDXGISwapChain* getSwapChainPointer() {
        // Offset 0x401C828U is also an option.
        return *(IDXGISwapChain**) ( getModuleBase() + 0x3FCF6D8U );
    }

}

// extern "C" {
//     void loadGameDLLHook();
//     uint64_t loadGameDLLHook_return;
//     HMODULE loadGameDLL(
//         LPCWSTR lpLibFileName,
//         HANDLE  hFile,
//         DWORD   dwFlags
//     );
// }

// HMODULE loadGameDLL(
//     LPCWSTR lpLibFileName,
//     HANDLE  hFile,
//     DWORD   dwFlags
// ) {
//     std::wcout << L"Loading library: " << lpLibFileName << L"\n";
//     return LoadLibraryExW( lpLibFileName, hFile, dwFlags );
// }

// namespace HaloMCC {

//     std::vector<HookPointer> hooks;

//     void initHooks() {

//         uint64_t mccBase = getModuleBase();
//         if ( !mccBase ) {
//             std::cout << "Could not find mcc-win64-shipping.exe module.\n";
//             return;
//         }

//         hooks.emplace_back( std::make_unique<Hook::JumpHook>(
//             "Load Game DLL Hook",
//             mccBase + 0x42F466U, 6,
//             (UINT_PTR) loadGameDLLHook,
//             loadGameDLLHook_return
//         ) );

//     }

//     void cleanupHooks() {
//         hooks.clear();
//     }

// }
