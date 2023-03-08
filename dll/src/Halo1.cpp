#include "headers/Halo1.h"
#include "graphics/headers/DX11Utils.h"
#include "utils/headers/Vec.h"
#include "utils/headers/common.h"
#include "utils/headers/AllocationUtils.h"

namespace Halo1 {

    static UINT_PTR dllBase;

    void init( UINT_PTR _dllBase ) { dllBase = _dllBase; }

    // === Methods ===

    Entity* EntityRecord::entity() { return getEntityPointer( this ); }

    char* Tag::getResourcePath() { return (char*) translateMapAddress( resourcePathAddress ); }
    void* Tag::getData() { return (void*) translateMapAddress( dataAddress ); }

    Tag* Entity::tag() { return Halo1::getTag( tagID ); }
    char* Entity::getTagResourcePath() {
        auto pTag = tag();
        if ( !pTag ) return nullptr;
        return pTag->getResourcePath();
    };
    bool Entity::fromResourcePath( const char* str ) {
        auto resourcePath = getTagResourcePath();
        return resourcePath && strncmp( resourcePath, str, 1024 ) == 0;
    }

    // ===============

    // === Pointers ===

    // static const UINT_PTR pDeviceContainerOffset = 0x2FA0D68;
    static const UINT_PTR entityArrayOffset = 0x2F01D80;
    static const UINT_PTR pEntityListOffset = 0x1DBE628;
    static const UINT_PTR playerCamOffset = 0x2F00D64;
    static const UINT_PTR playerHandleOffset = 0x1DD27D0;

    // DeviceContainer* getDeviceContainerPointer() { return *(DeviceContainer**) ( dllBase + pDeviceContainerOffset ); }
    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase + pEntityListOffset ); }
    UINT_PTR getEntityArrayBase() { return *(UINT_PTR*) ( dllBase + entityArrayOffset ); }
    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase + playerCamOffset ); }
    uint32_t getPlayerHandle() { return *(uint32_t*) ( dllBase + playerHandleOffset ); }

    uint64_t translateMapAddress( uint32_t address ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase + 0x2F01D98U );
        uint64_t mapBase = *(uint64_t*) ( dllBase + 0x3008370U );
        return address + ( relocatedMapBase - mapBase );
    }

    Tag* getTag( uint32_t tagID ) {
        Tag* tagArray = *(Tag**) ( dllBase + 0x1DB1390U );
        return &tagArray[tagID & 0xFFFF];
    }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle ) {
        if ( entityHandle == 0xFFFFFFFF )
            return nullptr;
        uint32_t i = entityHandle & 0xFFFF;
        auto recordAddress = (UINT_PTR) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
        return (EntityRecord*) recordAddress;
    }
    EntityRecord* getEntityRecord( uint32_t entityHandle ) { return getEntityRecord( getEntityListPointer(), entityHandle ); }

    void foreachEntityRecordIndexed( std::function<void( EntityRecord*, uint16_t i )> cb ) {
        auto pEntityList = getEntityListPointer();
        if ( !pEntityList )
            return;
        for ( uint16_t i = 0; i < pEntityList->capacity; i++ ) {
            auto pRecord = getEntityRecord( pEntityList, i );
            if ( pRecord->entityArrayOffset == -1 )
                continue;
            cb( pRecord, i );
        }
    }

    void foreachEntityRecord( std::function<void( EntityRecord* )> cb ) {
        foreachEntityRecordIndexed( [&cb]( EntityRecord* rec, uint16_t i ) { cb( rec ); } );
    }

    Entity* getEntityPointer( EntityRecord* pRecord ) {
        if ( !pRecord || pRecord->entityArrayOffset == -1 )
            return nullptr;
        UINT_PTR entityAddress = getEntityArrayBase() + 0x34 + (INT_PTR) pRecord->entityArrayOffset;
        return (Entity*) entityAddress;
    }

    Entity* getEntityPointer( uint32_t entityHandle ) {
        return getEntityPointer( getEntityRecord( entityHandle ) );
    }

    uint32_t indexToEntityHandle( uint16_t index ) {
        auto rec = getEntityRecord( index );
        if ( !rec ) return 0xFFFFFFFF;
        return rec->id << 16 | index;
    }

    // ================

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
            // || isPlayerHandle( entity->vehicleRiderHandle )
            // || isPlayerHandle( entity->controllerHandle )
            // || isPlayerHandle( entity->projectileParentHandle )
            // || rec->typeId == 0x0454
            ;
    }

    bool isReloading( Entity* entity ) { return entity->weaponAnim == 0x05; }
    bool isDoingMelee( Entity* entity ) { return entity->weaponAnim == 0x07; }

    bool isTransport( Entity* entity ) {
        return
            entity->fromResourcePath( "vehicles\\pelican\\pelican" ) ||
            entity->fromResourcePath( "vehicles\\c_dropship\\c_dropship" );
    }

    bool isRidingTransport( Entity* entity ) {
        if ( !entity )
            return false;
        auto vehicleRec = getEntityRecord( entity->parentHandle );
        if ( !vehicleRec )
            return false;
        auto vehicle = getEntityPointer( vehicleRec );
        return isTransport( vehicle );
    }

    // ======================

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
            foreachEntityRecord( []( EntityRecord* rec ) { printEntity( rec ); } );
        }
    }

    // ======================

    // === Camera Helpers ===

    // Theses values were found by trial and error using the updateFloat function (common.cpp).
    // For some reason, Halo 1's camera stores a different fov value than it actually uses and must be scaled. 
    float fovScale = 0.643564f;
    float clippingNear = 0.00782943f;
    float clippingFar = 386.369f;

    HRESULT getCameraMatrix( float w, float h, XMMATRIX& result ) {
        auto pCam = Halo1::getPlayerCameraPointer();
        if ( !pCam )
            return E_FAIL;
        result = cameraMatrix(
            pCam->pos, pCam->fwd, pCam->fov * fovScale,
            // clippingNear, clippingFar,
            clippingFar, clippingNear, // We have to swap the near and far plane to cooperate with Halo's reversed depth buffers.
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

    // =======================

    bool isCameraLoaded() { return !isZero( Halo1::getPlayerCameraPointer() ); }
    bool isEntityListLoaded() { return AllocationUtils::isAllocated( (UINT_PTR) getEntityListPointer() ); }
    bool isEntityArrayLoaded() { return AllocationUtils::isAllocated( (UINT_PTR) getEntityArrayBase() ); }
    bool isGameLoaded() { return GetModuleHandleA( "halo1.dll" ) && Halo1::isCameraLoaded() && isEntityListLoaded() && isEntityArrayLoaded(); }

}