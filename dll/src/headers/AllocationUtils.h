#include "../pch.h"

namespace AllocationUtils {

    UINT_PTR findFreePageNear(UINT_PTR address, size_t minSize);

    UINT_PTR virtualAllocNear(UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect);

    void test_virtualAllocNear(UINT_PTR address, size_t size);
    
}


