#include "headers/HaloMCC.h"
#include "headers/Hook.h"

namespace HaloMCC {

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
        return (uint64_t) GetModuleHandleA( MCC_MODULE_NAME );
    }

    bool isInGame() {
        return *(bool*) ( getModuleBase() + 0x401EF70U );
    }

    IDXGISwapChain* getSwapChainPointer() {
        // Offset 0x401C828U is also an option.
        return *(IDXGISwapChain**) ( getModuleBase() + 0x3FCF6D8U );
    }

}
