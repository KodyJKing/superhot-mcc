#pragma once

#include <Windows.h>

namespace Memory {
    LPVOID virtualAllocateNear( uintptr_t address, size_t size, DWORD allocationType, DWORD protection );
}