#include "./headers/Halo1.h"
#include "../pch.h"

namespace Halo1 {
    UINT_PTR dllBase;

    void init( UINT_PTR _dllBase ) { dllBase = _dllBase; }

    UINT_PTR pDeviceContainerOffset = 0x2FA0D68;
    UINT_PTR playerCamOffset = 0x2F00D64;

    DeviceContainer* getDeviceContainerPointer() { return *(DeviceContainer**) ( dllBase + pDeviceContainerOffset ); }
    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase + playerCamOffset ); }
}