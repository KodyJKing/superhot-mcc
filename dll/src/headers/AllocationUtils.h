#include "../pch.h"

namespace AllocationUtils {

    UINT_PTR findFreePageNear(UINT_PTR address, size_t minSize);

    UINT_PTR VirtualAllocNear(UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect);
    
}


