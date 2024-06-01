#pragma once
#include <Windows.h>

namespace Overlay {
        void initializeContext(HWND targetWindow);
        void render();
        void init();
        void free();
}