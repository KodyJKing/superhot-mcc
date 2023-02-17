#include "headers/HaloMCC.h"

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

}