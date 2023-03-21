#include "../../../pch.h"

namespace AllocationUtils {

    bool isAllocated( UINT_PTR address );

    UINT_PTR findFreePageNear( UINT_PTR address, size_t minSize );

    UINT_PTR virtualAllocNear( UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect );

    void test_virtualAllocNear( UINT_PTR address, size_t size );

}
