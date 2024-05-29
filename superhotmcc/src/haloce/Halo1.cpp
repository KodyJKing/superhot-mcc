#include "Halo1.hpp"
#include <Windows.h>
#include <iostream>

namespace {

    bool isAllocated( uintptr_t address ) {
        MEMORY_BASIC_INFORMATION mbi;
        if ( !VirtualQuery( (LPCVOID) address, &mbi, sizeof( mbi ) ) )
            return false;
        return mbi.State == MEM_COMMIT;
    }

}

namespace Halo1 {

    static uintptr_t dllBase;

    void init() {
        dllBase = (uintptr_t) GetModuleHandleA( "halo1.dll" );
    }

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

    static const uintptr_t entityArrayOffset = 0x2D9CDF8;
    static const uintptr_t pEntityListOffset = 0x1C42248;
    // static const uintptr_t playerCamOffset = 0x2F00D64;    // Old offset
    static const uintptr_t playerHandleOffset = 0x1DD27D0; // Old offset

    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase + pEntityListOffset ); }
    uintptr_t getEntityArrayBase() { return *(uintptr_t*) ( dllBase + entityArrayOffset ); }
    // Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase + playerCamOffset ); }
    uint32_t getPlayerHandle() { 
        std::cout << "Error: getPlayerHandle has not been updated" << std::endl;
        Beep( 750, 300 );
        Beep( 750, 300 );

        return *(uint32_t*) ( dllBase + playerHandleOffset ); 
    }
    char* getMapName() {
        std::cout << "Error: getMapName has not been updated" << std::endl;
        Beep( 750, 300 );
        Beep( 750, 300 );

        char** stringPtr = (char**) ( dllBase + 0x02C81358U ); // Old offset
        if ( !isAllocated( (uintptr_t) stringPtr ) )
            return nullptr;

        char* result = ( *stringPtr ) + 0x0C;
        if ( !isAllocated( (uintptr_t) result ) )
            return nullptr;

        return result;
    }

    bool isOnMap( const char* mapName ) {
        auto actualMapName = getMapName();
        if ( !actualMapName )
            return false;
        return strncmp( mapName, actualMapName, strnlen( mapName, 256 ) ) == 0;
    }

    uint64_t translateMapAddress( uint32_t address ) {
        // uint64_t relocatedMapBase = *(uint64_t*) ( dllBase + 0x2F01D98U ); // Validate update and remove me.
        // uint64_t mapBase = *(uint64_t*) ( dllBase + 0x3008370U );          // Validate update and remove me.
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase + 0x2D9CE10U );
        uint64_t mapBase = *(uint64_t*) ( dllBase + 0x2EA3410U );
        return address + ( relocatedMapBase - mapBase );
    }

    Tag* getTag( uint32_t tagID ) {
        // Tag* tagArray = *(Tag**) ( dllBase + 0x1DB1390U ); // Validate update and remove me.
        Tag* tagArray = *(Tag**) ( dllBase + 0x1C34FB0U );
        return &tagArray[tagID & 0xFFFF];
    }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle ) {
        if ( entityHandle == 0xFFFFFFFF )
            return nullptr;
        uint32_t i = entityHandle & 0xFFFF;
        auto recordAddress = (uintptr_t) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
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
        uintptr_t entityAddress = getEntityArrayBase() + 0x34 + (INT_PTR) pRecord->entityArrayOffset;
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

    EntityRecord* getPlayerRecord() {
        auto rec = getEntityRecord( getPlayerHandle() );
        if ( !rec || !rec->entity() )
            return nullptr;
        return rec;
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
        std::cout << pEntity->pos.toString() << std::endl;
        std::cout << "Type ID: " << pRecord->typeId;
        std::cout << ", Health: " << pEntity->health;
        std::cout << ", Shield: " << pEntity->shield << std::endl << std::endl;
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

    // =======================

    bool isEntityListLoaded() { return isAllocated( (uintptr_t) getEntityListPointer() ); }
    bool isEntityArrayLoaded() { return isAllocated( (uintptr_t) getEntityArrayBase() ); }
    bool isGameLoaded() { return GetModuleHandleA( "halo1.dll" ) && /*Halo1::isCameraLoaded() &&*/ isEntityListLoaded() && isEntityArrayLoaded(); }

}