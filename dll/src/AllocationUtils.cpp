#include "./headers/AllocationUtils.h"

#define UPPERHEX std::uppercase << std::hex

using std::cout;
using std::endl;

namespace AllocationUtils {

    UINT_PTR findFreePageNear(UINT_PTR address, size_t minSize) {
        MEMORY_BASIC_INFORMATION memInfo;
        const size_t _MEM_INFO_SIZE = sizeof(MEMORY_BASIC_INFORMATION);

        UINT_PTR currentAddress = address + MININT32;
        cout << "Starting search from " << UPPERHEX << currentAddress << endl;

        while (true) {
            cout << UPPERHEX << currentAddress << endl;

            auto resultBytes = VirtualQuery((LPVOID) currentAddress, &memInfo, _MEM_INFO_SIZE);
            if (resultBytes != _MEM_INFO_SIZE)
                return 0;

            if ( (memInfo.State == MEM_FREE) && (memInfo.RegionSize >= minSize) ) {
                cout << "Found suitable page at " << UPPERHEX << (UINT_PTR) memInfo.BaseAddress << endl;
                cout << UPPERHEX << memInfo.AllocationBase << endl;
                cout << UPPERHEX << memInfo.AllocationProtect << endl;
                cout << UPPERHEX << memInfo.PartitionId << endl;
                cout << UPPERHEX << memInfo.Protect << endl;
                cout << UPPERHEX << memInfo.RegionSize << endl;
                cout << UPPERHEX << memInfo.Type << endl;
                return (UINT_PTR) memInfo.BaseAddress;
            }

            currentAddress += memInfo.RegionSize;
            if (currentAddress - address > MAXINT32)
                return 0;
        }

    }

    UINT_PTR VirtualAllocNear(UINT_PTR address, size_t size, DWORD flAllocationType, DWORD flProtect) {

        UINT_PTR allocAddress = findFreePageNear(address, size);

        if (allocAddress == 0)
            throw std::runtime_error("Could not find free page near requested address.");
            
        return (UINT_PTR) VirtualAlloc((LPVOID) allocAddress, size, flAllocationType, flProtect);

    }

}
