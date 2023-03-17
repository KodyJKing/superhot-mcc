#pragma once

#include "../../pch.h"

#define MCC_WINDOW_NAME "Halo: The Master Chief Collection  "
#define MCC_WINDOW_CLASS_NAME "UnrealWindow"
#define MCC_MODULE_NAME "mcc-win64-shipping.exe"

namespace HaloMCC {
    HWND getWindow();
    Vec2 getWindowSize();
    bool isInForeground();
    bool isInGame();
    IDXGISwapChain* getSwapChainPointer();
}