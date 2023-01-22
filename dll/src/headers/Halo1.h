#include "../../pch.h"
#include "../utils/headers/Vec3.h"

namespace Halo1 {

    // Created with ReClass.NET 1.2 by KN4CK3R

    class DeviceContainer {
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

    class Camera {
    public:
        Vec3 pos; //0x0000
        char pad_000C[16]; //0x000C
        uint32_t N00000163; //0x001C
        Vec3 fwd; //0x0020
        Vec3 up; //0x002C
        float fov; //0x0038
    }; //Size: 0x003C

    void init( UINT_PTR _dllBase );

    Camera* getPlayerCameraPointer();
    DeviceContainer* getDeviceContainerPointer();

}