#include <Windows.h>
#include <iostream>
#include "Console.hpp"

namespace Console {
    FILE *f;

    void alloc() {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        SetConsoleTitleA("SuperHot MCC");

        // Hide close button on console window (closing the console seems to close/crash the game)
        HWND consoleWindow = GetConsoleWindow();
        HMENU hMenu = GetSystemMenu(consoleWindow, FALSE);
        EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    }

    void free() {
        fclose(f);
        FreeConsole();
    }
}