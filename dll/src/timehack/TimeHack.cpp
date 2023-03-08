#include "headers/TimeHack.h"
#include "headers/TimeScale.h"
#include "headers/Rewind.h"
#include "../headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/common.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;

static const float speedLimit = 1.4f; // 0.834f;

// Deadzoning is intended to prevent discrete actions (like spawning projectiles) from being spammed when an entity should be nearly frozen.
static const float timescaleUpdateDeadzone = 0.05f;

// To allow for some discrete updates to still happen when deadzoned, we can update with a probability equal to the current timescale.
static const bool allowRandomUpdatesInDeadzone = false;

static bool freezeTimeEnabled = false;
static bool superhotEnabled = true;
static bool speedLimitEnabled = true;

static uint64_t runUntil = 0;

extern "C" {
    void     preEntityUpdate( uint32_t entityHandle );
    bool     preEntityUpdate_doUpdate;
    void     preEntityUpdateHook();
    uint64_t preEntityUpdateHook_return;
    uint64_t preEntityUpdateHook_end;

    void     postEntityUpdate( uint32_t entityHandle );
    void     postEntityUpdateHook();
    uint64_t postEntityUpdateHook_return;

    void fireRateFixHook();
    uint64_t fireRateFixHook_return;
}

float globalTimescale() {
    if ( !superhotEnabled )
        return 1.0f;
    return TimeScale::timescale;
}

float timescaleForEntity( EntityRecord* rec ) {
    // Don't timescale player's 
    if ( isPlayerControlled( rec ) )
        return 1.0f;

    auto entity = getEntityPointer( rec );

    if ( isTransport( entity ) )
        return 1.0f;

    if (
        entity->fromResourcePath( "levels\\b40\\devices\\b40_lift4800\\b40_lift4800" ) ||
        entity->fromResourcePath( "levels\\c10\\devices\\lift\\lift" ) ||
        entity->fromResourcePath( "levels\\c20\\devices\\platform\\platform" )
        )
        return 1.0f;

    if ( entity->entityCategory == EntityCategory_SoundScenery )
        return 1.0f;

    auto vehicleRec = getEntityRecord( entity->parentHandle );
    if ( vehicleRec ) {
        auto vehicle = getEntityPointer( vehicleRec );
        if ( isTransport( vehicle ) )
            return 1.0f;
    }

    return globalTimescale();
}

bool isSingleStepping() {
    return GetTickCount64() < runUntil;
}

bool shouldEntityUpdate( EntityRecord* rec ) {
    auto entity = getEntityPointer( rec );

    auto personalTimescale = timescaleForEntity( rec );

    uint16_t cat = entity->entityCategory;
    bool canDeadzone = cat != EntityCategory_Projectile && cat != EntityCategory_Vehicle;

    if ( canDeadzone && allowRandomUpdatesInDeadzone ) {
        float u = MathUtils::randf();
        if ( u < personalTimescale )
            canDeadzone = false;
    }

    bool canFreeze = !isPlayerControlled( rec );
    bool shouldFreeze =
        ( freezeTimeEnabled && !isSingleStepping() ) ||
        ( superhotEnabled && canDeadzone && personalTimescale <= timescaleUpdateDeadzone );
    return !canFreeze || !shouldFreeze;
}

int updateDepth = 0;
void preEntityUpdate( uint32_t entityHandle ) {

    static bool warnedRecursiveUpdate = false;
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

    Rewind::rewind( rec, timescaleForEntity( rec ), globalTimescale() );

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

        // Gives upper limit to weapons without ROF upper limit.
        auto fireRateFixHook_start = halo1Base + 0xBDEF90U;
        //
        ( new Hook::JumpHook(
            "Fire Rate Fix Hook",
            fireRateFixHook_start, 22,
            (UINT_PTR) fireRateFixHook,
            fireRateFixHook_return
        ) )->hook();
        // std::cout << "fireRateFixHook: " << (uint64_t) fireRateFixHook << "\n";

    }

    void onDllThreadUpdate() {
        toggleOption( "Freeze Time", freezeTimeEnabled, VK_F2 );
        toggleOption( "SUPERHOT", superhotEnabled, VK_F3 );
        toggleOption( "Speed Limit", speedLimitEnabled, VK_NUMPAD9 );
        if ( keypressed( VK_F1 ) )
            runUntil = GetTickCount64() + 100;
    }

    float timeElapsed = 0.0f;

    void onGameThreadUpdate() {
        TimeScale::update();
        float dt = ( freezeTimeEnabled && !isSingleStepping() ) ? 0.0f : ( superhotEnabled ? TimeScale::timescale : 1.0f );
        timeElapsed += dt * 1000 / 60.0f;
        // static auto i = 0u;
        // if ( i++ % 60 == 0 )
        //     std::cout << "Time elapsed: " << timeElapsed << "\n";
        // std::cout << std::setprecision( 4 ) << TimeScale::timescale << "\n";
    }

}