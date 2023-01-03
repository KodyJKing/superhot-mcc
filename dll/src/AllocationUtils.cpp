#include "./headers/AllocationUtils.h"
#include "./utils/headers/MathUtils.h"

#define UPPERHEX std::uppercase << std::hex

const size_t _MEM_INFO_SIZE = sizeof(MEMORY_BASIC_INFORMATION);

using std::cout;
using std::endl;

namespace AllocationUtils {

    size_t getPageSize() {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
    }

    size_t roundUpToPageSize(size_t bytes) {
        auto pageSize = getPageSize();
        // cout << "System page size: " << UPPERHEX << pageSize << endl;
        auto pages = bytes / pageSize;
        if (bytes % pageSize > 0)
            pages++;
        return pages * pageSize;
    }

    UINT_PTR findFreePageNear(UINT_PTR address, size_t minSize) {
        MEMORY_BASIC_INFORMATION memInfo;

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        const UINT_PTR offsetPadding =  sysInfo.dwAllocationGranularity + 30000;
        UINT_PTR minOffset = MININT32 + offsetPadding;
        UINT_PTR maxOffset = MAXINT32 - offsetPadding;
        UINT_PTR currentAddress = address + minOffset;
        // cout << "Starting search from " << UPPERHEX << currentAddress << endl;

        while (true) {
            // cout << UPPERHEX << currentAddress << endl;

            auto resultBytes = VirtualQuery((LPVOID) currentAddress, &memInfo, _MEM_INFO_SIZE);
            if (resultBytes != _MEM_INFO_SIZE)
                return 0;

            if ( (memInfo.State == MEM_FREE) && (memInfo.RegionSize >= minSize) ) {
                // cout << "Found suitable page at " << UPPERHEX << (UINT_PTR) memInfo.BaseAddress << endl;
                return (UINT_PTR) memInfo.BaseAddress;
            }

            currentAddress = (UINT_PTR) memInfo.BaseAddress + memInfo.RegionSize;
            if (currentAddress - address > maxOffset)
                return 0;
        }

    }

    UINT_PTR scanForFreePage(UINT_PTR address, size_t minSize, int direction) {
        
        MEMORY_BASIC_INFORMATION memInfo;
        UINT_PTR currentAddress = address;

        while (true) {
            
            auto resultBytes = VirtualQuery((LPVOID) currentAddress, &memInfo, _MEM_INFO_SIZE);
            if (resultBytes != _MEM_INFO_SIZE)
                return 0;
                
            if ( (memInfo.State == MEM_FREE) && (memInfo.RegionSize >= minSize) )
                return (UINT_PTR) memInfo.BaseAddress;
                
            if (direction > 0)
                currentAddress = (UINT_PTR) memInfo.BaseAddress + memInfo.RegionSize;
            else
                currentAddress = (UINT_PTR) memInfo.BaseAddress - 1;
                
            auto currentOffset = MathUtils::signedDifference(currentAddress, address);
            if (currentOffset > MAXINT32 || currentOffset < MININT32)
                return 0;
                
        }
        
    }
    
    UINT_PTR findFreePageNear2(UINT_PTR address, size_t minSize) {
        auto backAddress = scanForFreePage(address, minSize, -1);
        auto forwardAddress = scanForFreePage(address, minSize, 1);
        auto backDist = llabs( address - backAddress );
        auto forwardDist = llabs( address - backAddress );
        return backDist < forwardDist ? backAddress : forwardAddress;
    }

    UINT_PTR virtualAllocNear(UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect) {
        
        // auto oldSize = size;
        size = roundUpToPageSize(size);
        // cout << "Rounding size " << UPPERHEX << oldSize << " to " << UPPERHEX << size << endl;

        UINT_PTR allocAddress = findFreePageNear(address, size);
        // cout << "Intended allocation address: " << allocAddress << endl;

        if (allocAddress == 0)
            throw std::runtime_error("Could not find free page near requested address.");

        auto result = (UINT_PTR) VirtualAlloc((LPVOID) allocAddress, size, flAllocationType, flProtect);
        if (result == 0)
            throw std::runtime_error("Could not allocate.");
        
        return result;

    }

    void test_virtualAllocNear(UINT_PTR address, size_t size) {
        const UINT_PTR allocAddress = AllocationUtils::virtualAllocNear(
            address,
            size,
            MEM_RESERVE|MEM_COMMIT,
            PAGE_READWRITE
        );
        if (!allocAddress) {
            std::cout << "Could not allocate." << std::endl;
            std::cout << "Error code: " << GetLastError() << std::endl;
        } else {
            std::cout << "Allocated at: ";
            std::cout << std::uppercase << std::hex << allocAddress << std::endl;
            VirtualFree( (LPVOID) allocAddress, 0, MEM_RELEASE);
        }
    }

}
