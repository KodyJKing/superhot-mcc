#include "./headers/Halo1.h"
#include "../pch.h"

namespace Halo1 {
    UINT_PTR dllBase;

    void init( UINT_PTR _dllBase ) { dllBase = _dllBase; }

    UINT_PTR pDeviceContainerOffset = 0x2FA0D68;
    UINT_PTR entityArrayOffset = 0x2F01D80;
    UINT_PTR pEntityListOffset = 0x1DBE628;
    UINT_PTR playerCamOffset = 0x2F00D64;

    DeviceContainer* getDeviceContainerPointer() { return *(DeviceContainer**) ( dllBase + pDeviceContainerOffset ); }
    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase + pEntityListOffset ); }
    UINT_PTR getEntityArrayBase() { return *(UINT_PTR*) ( dllBase + entityArrayOffset ); }
    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase + playerCamOffset ); }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t i ) {
        auto recordAddress = (UINT_PTR) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
        return (EntityRecord*) recordAddress;
    }
    EntityRecord* getEntityRecord( uint32_t i ) { return getEntityRecord( getEntityListPointer(), i ); }

    void foreachEntityRecord( EntityListEntryCallback cb ) {
        auto pEntityList = getEntityListPointer();
        if ( !pEntityList )
            return;
        for ( uint32_t i = 0; i < pEntityList->count; i++ )
            if ( !cb( getEntityRecord( pEntityList, i ) ) )
                break;
    }

    Entity* getEntityPointer( EntityRecord* pRecord ) {
        if ( !pRecord )
            return nullptr;
        UINT_PTR entityAddress = getEntityArrayBase() + 0x34 + pRecord->entityArrayOffset;
        return (Entity*) entityAddress;
    }

}