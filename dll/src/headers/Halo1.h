#include "../../pch.h"

namespace Halo1 {

    class DeviceContainer {
        // Created with ReClass.NET 1.2 by KN4CK3R
    public:
        uint64_t vftable; //0x0000
        char gpuName[256]; //0x0008
        char pad_0108[992]; //0x0108
        ID3D11Device* pDevice; //0x04E8
        char pad_04F0[88]; //0x04F0
        uint32_t resolutionX; //0x0548
        uint32_t resolutionY; //0x054C
        char pad_0550[984]; //0x0550
    }; //Size: 0x0928

    void init( UINT_PTR _dllBase );
    DeviceContainer* getDeviceContainerPointer();

}