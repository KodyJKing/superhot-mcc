#include "headers/TimeHack.h"
#include "../headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/common.h"

using namespace Halo1;

extern "C" {
    void entityUpdateHook();
    uint64_t entityUpdateHook_return;
    uint64_t entityUpdateHook_end;

    bool entityUpdateHook_shouldUpdate( uint32_t entityHandle );
}

bool freezeTime = false;

bool isPlayerHandle( uint32_t entityHandle ) {
    auto rec = getEntityRecord( entityHandle );
    return rec && rec->typeId == TypeID_Player;
}

bool isPlayerControlled( uint32_t entityHandle ) {
    auto rec = getEntityRecord( entityHandle );
    if ( !rec ) return false;
    auto entity = getEntityPointer( rec );
    if ( !entity ) return false;

    return rec->typeId == TypeID_Player
        || isPlayerHandle( entity->parentHandle )
        || isPlayerHandle( entity->vehicleRiderHandle )
        // || isPlayerHandle( entity->controllerHandle )
        // || isPlayerHandle( entity->projectileParentHandle )
        // || rec->typeId == 0x0454
        ;
}

bool entityUpdateHook_shouldUpdate( uint32_t entityHandle ) {
    if ( !freezeTime )
        return true;

    return isPlayerControlled( entityHandle );

    // auto pRec = getEntityRecord( entityHandle );
    // if ( !pRec ) return true;
    // return pRec->typeId == TypeID_Player;

    // auto pEntity = getEntityPointer( pRec );
    // if ( !pEntity ) return true;
    // return pEntity && pEntity->health <= 0;
}

namespace TimeHack {

    void init( uint64_t halo1Base ) {
        std::cout << "Initializing time hack.\n";

        entityUpdateHook_end = halo1Base + 0xB898D2U;

        ( new Hook::SimpleJumpHook(
            "Freeze time",
            halo1Base + 0xB898A4U, 10,
            (UINT_PTR) entityUpdateHook,
            entityUpdateHook_return
        ) )->hook();

    }

    void onDllThreadUpdate() {
        toggleOption( "Freeze Time", freezeTime, VK_F2 );
    }

}