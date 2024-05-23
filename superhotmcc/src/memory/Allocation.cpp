#include <Windows.h>

namespace Memory {

    const size_t _MEM_INFO_SIZE = sizeof( MEMORY_BASIC_INFORMATION );

    inline size_t getPageSize() {
        SYSTEM_INFO sysInfo;
        GetSystemInfo( &sysInfo );
        return sysInfo.dwPageSize;
    }

    inline size_t roundUpToPageSize( size_t bytes ) {
        auto pageSize = getPageSize();
        auto pages = bytes / pageSize;
        if ( bytes % pageSize > 0 )
            pages++;
        return pages * pageSize;
    }

    inline uintptr_t roundDown( uintptr_t x, uintptr_t granularity ) {
        return ( x / granularity ) * granularity;
    }

    inline uintptr_t roundUp( uintptr_t x, uintptr_t granularity ) {
        auto result = ( x / granularity ) * granularity;
        if ( x % granularity > 0 )
            result += granularity;
        return result;
    }

    uintptr_t scanForFreePage( uintptr_t address, size_t minSize, int direction ) {
        MEMORY_BASIC_INFORMATION memInfo;
        SYSTEM_INFO sysInfo;

        GetSystemInfo( &sysInfo );

        uintptr_t currentAddress = roundUp( address, sysInfo.dwAllocationGranularity );

        while ( true ) {

            auto resultBytes = VirtualQuery( (LPVOID) currentAddress, &memInfo, _MEM_INFO_SIZE );
            if ( resultBytes != _MEM_INFO_SIZE )
                return 0;

            if ( ( memInfo.State == MEM_FREE ) && ( memInfo.RegionSize >= minSize ) )
                return (uintptr_t) memInfo.BaseAddress;

            if ( direction > 0 )
                currentAddress = roundUp(
                    (uintptr_t) memInfo.BaseAddress + memInfo.RegionSize,
                    sysInfo.dwAllocationGranularity
                );
            else
                currentAddress = roundDown(
                    (uintptr_t) memInfo.BaseAddress - 1,
                    sysInfo.dwAllocationGranularity
                );

            intptr_t currentOffset = currentAddress - address;
            if ( currentOffset > MAXINT32 || currentOffset < MININT32 )
                return 0;
        }
    }

    uintptr_t findFreePageNear( uintptr_t address, size_t minSize ) {
        auto backAddress = scanForFreePage(address, minSize, -1 );
        auto forwardAddress = scanForFreePage(address, minSize, 1 );

        auto backDist = address > backAddress ? address - backAddress : backAddress - address;
        auto forwardDist = address > forwardAddress ? address - forwardAddress : forwardAddress - address;

        return backDist < forwardDist ? backAddress : forwardAddress;
    }

    LPVOID virtualAllocateNear( uintptr_t address, size_t size, DWORD allocationType, DWORD protection ) {
        auto freePage = findFreePageNear( address, size );
        return VirtualAlloc( (LPVOID) freePage, size, allocationType, protection );
    }

}