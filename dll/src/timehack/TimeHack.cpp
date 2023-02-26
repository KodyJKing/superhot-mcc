#include "headers/TimeHack.h"
#include "headers/TimeScale.h"
#include "headers/Rewind.h"
#include "../headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/common.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;

// Speed limiting is a hackish solution to projectiles linecasting too far and hitting a wall before they actually should.
// Preemptively scaling back velocity works for some projectile types, but not for projectiles which despawn when they reach their end speed (like plasma bolts).
const float speedLimit = 1.4f;

// Deadzoning is intended to prevent discrete actions (like spawning projectiles) from being spammed when an entity should be nearly frozen.
const float timescaleUpdateDeadzone = 0.05f;

// To allow for some discrete updates to still happen when deadzoned, we can update with a probability equal to the current timescale.
const bool allowRandomUpdatesInDeadzone = true;

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
}

bool shouldEntityUpdate( EntityRecord* rec ) {
    auto entity = getEntityPointer( rec );

    uint16_t cat = entity->entityCategory;
    bool canDeadzone = cat != EntityCategory_Projectile && cat != EntityCategory_Vehicle;

    if ( canDeadzone && allowRandomUpdatesInDeadzone ) {
        float u = MathUtils::randf();
        if ( u < TimeScale::timescale )
            canDeadzone = false;
    }

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

int updateDepth = 0;
bool warnedRecursiveUpdate;
void preEntityUpdate( uint32_t entityHandle ) {

    if ( updateDepth++ > 0 && !warnedRecursiveUpdate ) {
        std::cout << "Warning, recursive update detected!\n";
        warnedRecursiveUpdate = true;
    }

    EntityRecord* rec = getEntityRecord( entityHandle );
    Entity* entity = getEntityPointer( rec );
    if ( !entity )
        return;

    preEntityUpdate_doUpdate = shouldEntityUpdate( rec );

    if ( speedLimitEnabled )
        Vec::clampMut( entity->vel, speedLimit );

    Rewind::snapshot( rec );

}

void postEntityUpdate( uint32_t entityHandle ) {

    updateDepth--;

    EntityRecord* rec = getEntityRecord( entityHandle );
    Entity* entity = getEntityPointer( rec );
    if ( !entity )
        return;

    float globalTimescale = TimeScale::timescale;
    float personalTimescale = shouldRewind( rec ) ? globalTimescale : 1.0f;
    Rewind::rewind( rec, personalTimescale, globalTimescale );

}

namespace TimeHack {

    void init( uint64_t halo1Base ) {

        std::cout << "Initializing time hack.\n";

        auto preEntityUpdateHook_start = halo1Base + 0xB898A4U;
        preEntityUpdateHook_end = halo1Base + 0xB898D2U;
        //
        ( new Hook::JumpHook(
            "Pre Entity Update Hook",
            preEntityUpdateHook_start, 10,
            (UINT_PTR) preEntityUpdateHook,
            preEntityUpdateHook_return
        ) )->hook();

        auto posEntityUpdateHook_start = halo1Base + 0xB89A3BU;
        //
        ( new Hook::JumpHook(
            "Post Entity Update Hook",
            posEntityUpdateHook_start, 5,
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