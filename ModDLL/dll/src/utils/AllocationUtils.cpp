#include "./headers/AllocationUtils.h"
#include "./headers/MathUtils.h"

#define UPPERHEX std::uppercase << std::hex

const size_t _MEM_INFO_SIZE = sizeof( MEMORY_BASIC_INFORMATION );

using std::cout;
using std::endl;

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

inline UINT_PTR roundDown( UINT_PTR x, UINT_PTR granularity ) {
    return ( x / granularity ) * granularity;
}

inline UINT_PTR roundUp( UINT_PTR x, UINT_PTR granularity ) {
    auto result = ( x / granularity ) * granularity;
    if ( x % granularity > 0 )
        result += granularity;
    return result;
}

namespace AllocationUtils {

    bool isAllocated( UINT_PTR address ) {
        MEMORY_BASIC_INFORMATION memInfo;

        auto resultBytes = VirtualQuery( (LPVOID) address, &memInfo, _MEM_INFO_SIZE );
        if ( resultBytes != _MEM_INFO_SIZE )
            return false;

        return memInfo.State == MEM_COMMIT;
    }

    UINT_PTR scanForFreePage( UINT_PTR address, size_t minSize, int direction ) {
        MEMORY_BASIC_INFORMATION memInfo;
        SYSTEM_INFO sysInfo;

        GetSystemInfo( &sysInfo );

        UINT_PTR currentAddress = roundUp( address, sysInfo.dwAllocationGranularity );

        while ( true ) {

            auto resultBytes = VirtualQuery( (LPVOID) currentAddress, &memInfo, _MEM_INFO_SIZE );
            if ( resultBytes != _MEM_INFO_SIZE )
                return 0;

            if ( ( memInfo.State == MEM_FREE ) && ( memInfo.RegionSize >= minSize ) )
                return (UINT_PTR) memInfo.BaseAddress;

            if ( direction > 0 )
                currentAddress = roundUp(
                    (UINT_PTR) memInfo.BaseAddress + memInfo.RegionSize,
                    sysInfo.dwAllocationGranularity
                );
            else
                currentAddress = roundDown(
                    (UINT_PTR) memInfo.BaseAddress - 1,
                    sysInfo.dwAllocationGranularity
                );

            auto currentOffset = MathUtils::signedDifference( currentAddress, address );
            if ( currentOffset > MAXINT32 || currentOffset < MININT32 )
                return 0;

        }
    }

    UINT_PTR findFreePageNear( UINT_PTR address, size_t minSize ) {
        auto backAddress = scanForFreePage( address, minSize, -1 );
        auto forwardAddress = scanForFreePage( address, minSize, 1 );

        auto backDist = address > backAddress ? address - backAddress : backAddress - address;
        auto forwardDist = address > forwardAddress ? address - forwardAddress : forwardAddress - address;

        return backDist < forwardDist ? backAddress : forwardAddress;
    }

    UINT_PTR virtualAllocNear( UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect ) {

        // auto oldSize = size;
        size = roundUpToPageSize( size );
        // cout << "Rounding size " << UPPERHEX << oldSize << " to " << UPPERHEX << size << endl;

        UINT_PTR allocAddress = findFreePageNear( address, size );
        // cout << "Intended allocation address: " << allocAddress << endl;

        if ( allocAddress == 0 )
            throw std::runtime_error( "Could not find free page near requested address." );

        auto result = (UINT_PTR) VirtualAlloc( (LPVOID) allocAddress, size, flAllocationType, flProtect );
        if ( result == 0 ) {
            std::stringstream errorMessage;
            errorMessage << "Allocation failed with error code: " << GetLastError();
            throw std::runtime_error( errorMessage.str() );
        }

        return result;

    }

    void test_virtualAllocNear( UINT_PTR address, size_t size ) {
        const UINT_PTR allocAddress = AllocationUtils::virtualAllocNear(
            address,
            size,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE
        );
        if ( !allocAddress ) {
            std::cout << "Could not allocate." << std::endl;
            std::cout << "Error code: " << GetLastError() << std::endl;
        } else {
            std::cout << "Allocated at: ";
            std::cout << std::uppercase << std::hex << allocAddress << std::endl;
            VirtualFree( (LPVOID) allocAddress, 0, MEM_RELEASE );
        }
    }

}
