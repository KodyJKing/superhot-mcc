#include "headers/TimeHack.h"
#include "../headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/common.h"

using namespace Halo1;

extern "C" {
    void     preEntityUpdate( uint32_t entityHandle );
    bool     preEntityUpdate_doUpdate;
    void     preEntityUpdateHook();
    uint64_t preEntityUpdateHook_return;
    uint64_t preEntityUpdateHook_end;

    void     postEntityUpdate( uint32_t entityHandle );
    void     postEntityUpdateHook();
    uint64_t postEntityUpdateHook_return;
    uint64_t postEntityUpdateHook_jmp;
}

bool freezeTimeEnabled = false;
bool superhotEnabled = false;

bool shouldEntityUpdate( uint32_t entityHandle ) {
    if ( !freezeTimeEnabled )
        return true;

    return isPlayerControlled( entityHandle );
}

void preEntityUpdate( uint32_t entityHandle ) {

    preEntityUpdate_doUpdate = shouldEntityUpdate( entityHandle );

    // Save entity state

}

void postEntityUpdate( uint32_t entityHandle ) {

    // Rewind entity state

}

namespace TimeHack {

    void init( uint64_t halo1Base ) {

        std::cout << "Initializing time hack.\n";

        preEntityUpdateHook_end = halo1Base + 0xB898D2U;
        postEntityUpdateHook_jmp = halo1Base + 0xB898E0U;

        ( new Hook::SimpleJumpHook(
            "Pre Entity Update Hook",
            halo1Base + 0xB898A4U, 10,
            (UINT_PTR) preEntityUpdateHook,
            preEntityUpdateHook_return
        ) )->hook();

        ( new Hook::SimpleJumpHook(
            "Post Entity Update Hook",
            preEntityUpdateHook_end, 6,
            (UINT_PTR) postEntityUpdateHook,
            postEntityUpdateHook_return
        ) )->hook();

    }

    void onDllThreadUpdate() {
        toggleOption( "Freeze Time", freezeTimeEnabled, VK_F2 );
        toggleOption( "SUPERHOT", superhotEnabled, VK_F3 );
    }

}