#include "headers/Halo1.h"
#include "graphics/headers/DX11Utils.h"
#include "utils/headers/Vec.h"
#include "utils/headers/common.h"

namespace Halo1 {
    UINT_PTR dllBase;

    void init( UINT_PTR _dllBase ) { dllBase = _dllBase; }

    UINT_PTR pDeviceContainerOffset = 0x2FA0D68;
    UINT_PTR entityArrayOffset = 0x2F01D80;
    UINT_PTR pEntityListOffset = 0x1DBE628;
    UINT_PTR playerCamOffset = 0x2F00D64;
    UINT_PTR playerHandleOffset = 0x1DD27D0;

    DeviceContainer* getDeviceContainerPointer() { return *(DeviceContainer**) ( dllBase + pDeviceContainerOffset ); }
    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase + pEntityListOffset ); }
    UINT_PTR getEntityArrayBase() { return *(UINT_PTR*) ( dllBase + entityArrayOffset ); }
    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase + playerCamOffset ); }
    uint32_t getPlayerHandle() { return *(uint32_t*) ( dllBase + playerHandleOffset ); }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle ) {
        if ( entityHandle == 0xFFFFFFFF )
            return nullptr;
        uint32_t i = entityHandle & 0xFFFF;
        auto recordAddress = (UINT_PTR) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
        return (EntityRecord*) recordAddress;
    }
    EntityRecord* getEntityRecord( uint32_t entityHandle ) { return getEntityRecord( getEntityListPointer(), entityHandle ); }

    void foreachEntityRecord( EntityListEntryCallback cb ) {
        auto pEntityList = getEntityListPointer();
        if ( !pEntityList )
            return;
        for ( uint32_t i = 0; i < pEntityList->capacity; i++ ) {
            auto pRecord = getEntityRecord( pEntityList, i );
            if ( pRecord->entityArrayOffset == -1 )
                continue;
            if ( !cb( pRecord ) )
                break;
        }
    }

    Entity* getEntityPointer( EntityRecord* pRecord ) {
        if ( !pRecord || pRecord->entityArrayOffset == -1 )
            return nullptr;
        UINT_PTR entityAddress = getEntityArrayBase() + 0x34 + (INT_PTR) pRecord->entityArrayOffset;
        return (Entity*) entityAddress;
    }

    bool isPlayerHandle( uint32_t entityHandle ) {
        auto rec = getEntityRecord( entityHandle );
        return rec && rec->typeId == TypeID_Player;
    }

    bool isPlayerControlled( EntityRecord* rec ) {
        auto entity = getEntityPointer( rec );
        if ( !entity )
            return false;

        return rec->typeId == TypeID_Player
            || isPlayerHandle( entity->parentHandle )
            || isPlayerHandle( entity->vehicleRiderHandle )
            // || isPlayerHandle( entity->controllerHandle )
            // || isPlayerHandle( entity->projectileParentHandle )
            // || rec->typeId == 0x0454
            ;
    }

    bool isReloading( Entity* entity ) { return entity->weaponAnim == 0x05; }
    bool isDoingMelee( Entity* entity ) { return entity->weaponAnim == 0x07; }

    EntityType getEntityType( uint16_t typeId ) { return getEntityType( (TypeID) typeId ); }
    EntityType getEntityType( TypeID typeId ) {
        switch ( typeId ) {
            case TypeID_Player:     return { .name = L"Player",     .living = 1, .hostile = 0 };
            case TypeID_Marine:     return { .name = L"Marine",     .living = 1, .hostile = 0 };
            case TypeID_Jackal:     return { .name = L"Jackal",     .living = 1, .hostile = 1 };
            case TypeID_Grunt:      return { .name = L"Grunt",      .living = 1, .hostile = 1 };
            case TypeID_Elite:      return { .name = L"Elite",      .living = 1, .hostile = 1 };
            case TypeID_VehicleA:   return { .name = L"VehicleA",   .living = 1, .hostile = 1, .transport = 1 };
            case TypeID_VehicleB:   return { .name = L"VehicleB",   .living = 1, .hostile = 1, .transport = 1 };
            case TypeID_Projectile: return { .name = L"Projectile", .living = 0, .hostile = 0 };
        }
        return { .name = L"Unknown", .unknown = 1 };
    }

    bool printEntity( EntityRecord* pRecord ) {
        auto pEntity = getEntityPointer( pRecord );
        if ( !pEntity )
            return true;
        std::cout << "Position: ";
        Vec::print( pEntity->pos );
        std::cout << "\n";
        std::cout << "Type ID: " << pRecord->typeId;
        std::cout << ", Health: " << pEntity->health;
        std::cout << ", Shield: " << pEntity->shield << "\n\n";
        return true;
    }

    void printEntities() {
        auto pEntityList = getEntityListPointer();
        if ( pEntityList ) {
            std::cout << "Entity list at: " << pEntityList << "\n";
            std::cout << "Entities: \n\n";
            foreachEntityRecord( printEntity );
        }
    }

    // === Camera helpers ===

    // For some reason, Halo 1's camera stores a different fov value than it actually uses and must be scaled. 
    // This value was found by trial and error using the updateFloat function.
    float fovScale = 0.643564f;
    float clippingNear = 0.1f;
    float clippingFar = 100.0f;
    HRESULT getCameraMatrix( float w, float h, XMMATRIX& result ) {
        auto pCam = Halo1::getPlayerCameraPointer();
        if ( !pCam )
            return E_FAIL;
        result = cameraMatrix(
            pCam->pos, pCam->fwd, pCam->fov * fovScale,
            clippingNear, clippingFar,
            w, h
        );
        return S_OK;
    }

    Vec3 projectPoint( float w, float h, const Vec3 point ) {
        auto pCam = Halo1::getPlayerCameraPointer();
        if ( !pCam )
            return {};
        return worldToScreen(
            point,
            pCam->pos, pCam->fwd, pCam->fov * fovScale,
            clippingNear, clippingFar,
            w, h
        );
    }

    bool isCameraLoaded() {
        return !isZero( Halo1::getPlayerCameraPointer() );
    }

}