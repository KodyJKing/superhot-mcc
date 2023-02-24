#include "headers/TimeHack.h"
#include "headers/TimeScale.h"
#include "headers/Rewind.h"
#include "../headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/common.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;

const float speedLimit = 1.4f;
const float timescaleUpdateDeadzone = 0.05f;

bool freezeTimeEnabled = false;
bool superhotEnabled = true;
bool speedLimitEnabled = true;

uint64_t runUntil = 0;

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

bool shouldEntityUpdate( EntityRecord* rec ) {
    auto entity = getEntityPointer( rec );

    uint16_t cat = entity->entityCategory;
    bool canDeadzone = cat != EntityCategory_Projectile && cat != EntityCategory_Vehicle;

    bool canFreeze = !isPlayerControlled( rec );
    bool shouldFreeze =
        ( freezeTimeEnabled && GetTickCount64() >= runUntil ) ||
        ( superhotEnabled && canDeadzone && TimeScale::timescale <= timescaleUpdateDeadzone );
    return !canFreeze || !shouldFreeze;
}

bool shouldRewind( EntityRecord* rec ) {
    if ( !superhotEnabled ) // !superhotEnabled|| freezeTimeEnabled )
        return false;
    return !isPlayerControlled( rec );
}

void preEntityUpdate( uint32_t entityHandle ) {

    EntityRecord* rec = getEntityRecord( entityHandle );
    Entity* entity = getEntityPointer( rec );
    if ( !entity )
        return;

    preEntityUpdate_doUpdate = shouldEntityUpdate( rec );

    if ( speedLimitEnabled )
        Vec::clampMut( entity->vel, speedLimit );

    if ( shouldRewind( rec ) )
        Rewind::snapshot( rec );

}

void postEntityUpdate( uint32_t entityHandle ) {

    EntityRecord* rec = getEntityRecord( entityHandle );
    Entity* entity = getEntityPointer( rec );
    if ( !entity )
        return;

    if ( shouldRewind( rec ) )
        Rewind::rewind( rec, TimeScale::timescale );
    // Rewind::rewind( rec, 0.1f );

}

namespace TimeHack {

    void init( uint64_t halo1Base ) {

        std::cout << "Initializing time hack.\n";

        // 0FB7 FB           - movzx edi,bx
        // 48 8B 34 C6       - mov rsi,[rsi+rax*8]
        //
        // preEntityUpdateHook_start:
        // 48 8B 86 F0000000 - mov rax,[rsi+000000F0]
        // 48 85 C0          - test rax,rax
        //
        // 74 22             - je 7FFF465998D2
        // 48 8B 50 58       - mov rdx,[rax+58]
        //
        auto preEntityUpdateHook_start = halo1Base + 0xB898A4U;

        preEntityUpdateHook_end = halo1Base + 0xB898D2U;
        postEntityUpdateHook_jmp = halo1Base + 0xB898E0U;

        ( new Hook::JumpHook(
            "Pre Entity Update Hook",
            preEntityUpdateHook_start, 10,
            (UINT_PTR) preEntityUpdateHook,
            preEntityUpdateHook_return
        ) )->hook();

        ( new Hook::JumpHook(
            "Post Entity Update Hook",
            preEntityUpdateHook_end, 6,
            (UINT_PTR) postEntityUpdateHook,
            postEntityUpdateHook_return
        ) )->hook();

        TimeScale::init();

    }

    void onDllThreadUpdate() {
        toggleOption( "Freeze Time", freezeTimeEnabled, VK_F2 );
        toggleOption( "SUPERHOT", superhotEnabled, VK_F3 );
        toggleOption( "Speed Limit", speedLimitEnabled, VK_NUMPAD2 );
        if ( keypressed( VK_F1 ) )
            runUntil = GetTickCount64() + 100;
    }

    void onGameThreadUpdate() {
        TimeScale::update();
        // std::cout << std::setprecision( 4 ) << TimeScale::timescale << "\n";
    }

}