#pragma once
#include <Windows.h>
#include <optional>

namespace Memory {

    bool isAllocated(uintptr_t address);

    template<typename T>
    std::optional<T> safeRead(uintptr_t address) {
        T buffer;
        if(address && ReadProcessMemory(GetCurrentProcess(), (LPCVOID)address, &buffer, sizeof(T), NULL))
            return buffer;
        return std::nullopt;
    }

    //  Safe for unallocated memory, but not check page protection.
    template<typename T>
    bool safeWriteFast(uintptr_t address, T value) {
        if (isAllocated(address)) {
            T* ptr = (T*)address;
            *ptr = value;
            return true;
        }
        return false;
    }

    template<typename T>
    bool safeWrite(uintptr_t address, T value) {
        DWORD oldProtect;
        if (!VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
            return false;
        bool result = safeWriteFast(address, value);
        VirtualProtect((LPVOID)address, sizeof(T), oldProtect, &oldProtect);
        return result;
    }
    
}