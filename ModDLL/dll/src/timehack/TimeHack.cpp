#include "headers/TimeHack.h"
#include "headers/TimeScale.h"
#include "headers/Rewind.h"
#include "../utils/headers/Hook.h"
#include "../headers/Halo1.h"
#include "../utils/headers/BytePattern.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Config.h"
#include "../utils/headers/common.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;
using namespace Hook;
using std::make_unique;
using std::unique_ptr;

static float speedLimit = 0.834f; // 1.4f;

// Deadzoning is intended to prevent discrete actions (like spawning projectiles) from being spammed when an entity should be nearly frozen.
static float timescaleUpdateDeadzone = 0.05f;

// To allow for some discrete updates to still happen when deadzoned, we can update with a probability equal to the current timescale.
static bool allowRandomUpdatesInDeadzone = false;

static bool freezeTimeEnabled = false;
static bool superhotEnabled = true;
static bool speedLimitEnabled = true;

static float playerDamageScale = 3.0f;
static float npcDamageScale = 2.0f;

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

    float damageScaleForEntity( uint32_t entityHandle );
    uint64_t damageHealthHook_return;
    void damageHealthHook();

    uint64_t damageShieldHook_return;
    void damageShieldHook();
}

float damageScaleForEntity( uint32_t entityHandle ) {
    if ( !superhotEnabled )
        return 1.0f;

    EntityRecord* rec = getEntityRecord( entityHandle );
    Entity* entity = rec->entity();
    if ( !entity ) return 1.0f;

    if ( rec->typeId == TypeID_Player )
        return playerDamageScale;
    else
        return npcDamageScale;
}

float globalTimescale() {
    if ( !superhotEnabled )
        return 1.0f;
    return TimeScale::timescale;
}

float timescaleForEntity( EntityRecord* rec ) {
    auto entity = getEntityPointer( rec );

    if (
        isPlayerControlled( rec ) ||

        isTransport( entity ) ||
        entity->entityCategory == EntityCategory_SoundScenery ||

        // Don't timescale lifts. That's annoying.
        entity->fromResourcePath( "levels\\b40\\devices\\b40_lift1600\\b40_lift1600" ) ||
        entity->fromResourcePath( "levels\\b40\\devices\\b40_lift4800\\b40_lift4800" ) ||
        entity->fromResourcePath( "levels\\c10\\devices\\lift\\falling lift" ) ||
        entity->fromResourcePath( "levels\\c10\\devices\\lift\\lift" ) ||
        entity->fromResourcePath( "levels\\c20\\devices\\platform\\platform" ) ||
        entity->fromResourcePath( "levels\\a10\\devices\\elevator\\elevator" ) ||

        // Don't freeze Keys on Pilar of Autumn or the player will get stuck after their meeting.
        // (Keys has to give them the gun before they can move.)
        entity->fromResourcePath( "characters\\captain\\captain" )
        && isOnMap( "halo1\\maps\\a10.map" )
        )
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

static int updateDepth = 0;
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

    static std::vector<HookPointer> hooks;

    bool init( uint64_t halo1Base ) {

        hooks.clear();

        std::cout << "Initializing time hack.\n";

        bool instructionsCheck =
            assertBytes( "Pre Entity Update", halo1Base + 0xB898A0U, "48 8B 34 C6 48 8B 86 F0 00 00 00 48 85 C0" ) &&
            assertBytes( "Post Entity Update", halo1Base + 0xB89A34U, "33 DB 4C 8B 7C 24 20 48 8B 44 24 30 48 8B BC 24 C0 00 00 00 48 8B B4 24 B8 00 00 00" ) &&
            assertBytes( "Fire Rate Fix", halo1Base + 0xBDEF90U, "F3 0F 10 52 08 0F 57 DB F3 0F 5C 52 04 F3 0F 59 D1 F3 0F 58 52 04" ) &&
            assertBytes( "Damage Health", halo1Base + 0xC19090U, "F3 0F 10 83 9C 00 00 00 F3 0F 5C C6 F3 0F 11 83 9C 00 00 00" ) &&
            assertBytes( "Damage Shield", halo1Base + 0xC197D0U, "F3 0F 5C CA F3 0F 11 0F 41 F6 C1 02" );

        if ( !instructionsCheck ) {
            auto message = "Could not install time manipulation hooks. The game may have updated.";
            std::cout << message << "\n";
            MessageBoxA( NULL, message, "SuperHot MCC Error", MB_OK );
            return false;
        }

        auto preEntityUpdateHook_start = halo1Base + 0xB898A4U;
        preEntityUpdateHook_end = halo1Base + 0xB898D2U;
        //
        hooks.emplace_back( make_unique<JumpHook>(
            "Pre Entity Update Hook",
            preEntityUpdateHook_start, 10,
            (UINT_PTR) preEntityUpdateHook,
            preEntityUpdateHook_return
        ) );

        auto posEntityUpdateHook_start = halo1Base + 0xB89A3BU;
        //
        hooks.emplace_back( make_unique<JumpHook>(
            "Post Entity Update Hook",
            posEntityUpdateHook_start, 5,
            (UINT_PTR) postEntityUpdateHook,
            postEntityUpdateHook_return
        ) );

        // Gives upper limit to weapons without ROF upper limit.
        auto fireRateFixHook_start = halo1Base + 0xBDEF90U;
        //
        hooks.emplace_back( make_unique<JumpHook>(
            "Fire Rate Fix Hook",
            fireRateFixHook_start, 22,
            (UINT_PTR) fireRateFixHook,
            fireRateFixHook_return
        ) );

        hooks.emplace_back( make_unique<JumpHook>(
            "Damage Health Hook",
            halo1Base + 0xC19090U, 12,
            (UINT_PTR) damageHealthHook,
            damageHealthHook_return
        ) );

        hooks.emplace_back( make_unique<JumpHook>(
            "Damage Shield Hook",
            halo1Base + 0xC197D0U, 8,
            (UINT_PTR) damageShieldHook,
            damageShieldHook_return
        ) );

        std::cout << "Reading timehack configurations.\n";
        speedLimit = Config::getFloat( "gameplay", "speedLimit", 0.834f );
        timescaleUpdateDeadzone = Config::getFloat( "gameplay", "timescaleUpdateDeadzone", 0.05f );
        allowRandomUpdatesInDeadzone = (bool) Config::getUint64( "gameplay", "allowRandomUpdatesInDeadzone", false );
        playerDamageScale = Config::getFloat( "gameplay", "playerDamageScale", 3.0f );
        npcDamageScale = Config::getFloat( "gameplay", "npcDamageScale", 2.0f );

        TimeScale::init();

        return true;
    }

    void cleanup() {
        hooks.clear();
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