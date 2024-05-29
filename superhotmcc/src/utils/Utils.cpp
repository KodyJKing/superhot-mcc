#include <Windows.h>
#include <string>

namespace Utils {

    HMODULE waitForModule(std::string moduleName, DWORD sleepTime, DWORD timeout) {
        HMODULE module = nullptr;
        DWORD startTick = GetTickCount();
        while (!module) {
            module = GetModuleHandleA(moduleName.c_str());
            Sleep(sleepTime);
            if (timeout && GetTickCount() - startTick > timeout)
                return nullptr;
        }
        return module;
    }

}