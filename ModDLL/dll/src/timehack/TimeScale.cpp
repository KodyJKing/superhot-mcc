#include "headers/TimeScale.h"
#include "../utils/headers/Config.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;
using namespace MathUtils;

namespace TimeScale {

    static const float walkingSpeed = 0.07f;

    static float minTimescale = 0.05f;
    static float activityDecayRate = 0.05f;

    static float rotationActivityCoefficient = 100.0f;
    static float rotationSpeedSmoothing = 0.75f;
    static float maxTimescaleDueToTurning = 0.25f;

    static DWORD unpauseAfterFireMilis = 40;
    static DWORD unpauseReloadMilis = 750;
    static DWORD unpauseMeleeMilis = 250;
    static DWORD unpauseWeaponSwapMilis = 250;

    float timescale;
    static float activityLevel;
    static uint64_t playerIsActingUntil = 0;
    static Vec3 previousLook;
    static float lookSpeed, lookSpeedSmoothed;

    void unpauseForNMilis( uint64_t milis ) { playerIsActingUntil = GetTickCount() + milis; }

    void init() {
        minTimescale = Config::getFloat( "gameplay", "minTimeScale", 0.05f );
        activityDecayRate = Config::getFloat( "gameplay", "activityDecayRate", 0.05f );

        rotationActivityCoefficient = Config::getFloat( "gameplay", "rotationActivityCoefficient", 100.0f );
        rotationSpeedSmoothing = Config::getFloat( "gameplay", "rotationSpeedSmoothing", 0.75f );
        maxTimescaleDueToTurning = Config::getFloat( "gameplay", "maxTimescaleDueToTurning", 0.25f );

        unpauseAfterFireMilis = (DWORD) Config::getUint64( "gameplay", "unpauseAfterFireMilis", 40 );
        unpauseReloadMilis = (DWORD) Config::getUint64( "gameplay", "unpauseReloadMilis", 750 );
        unpauseMeleeMilis = (DWORD) Config::getUint64( "gameplay", "unpauseMeleeMilis", 250 );
        unpauseWeaponSwapMilis = (DWORD) Config::getUint64( "gameplay", "unpauseWeaponSwapMilis", 250 );

        timescale = minTimescale;
    }

    void updateLookSpeed() {
        auto pCam = getPlayerCameraPointer();
        float sinRot = Vec::length( Vec::cross( previousLook, pCam->fwd ) );
        lookSpeed = asinf( sinRot );
        lookSpeedSmoothed = lerp( lookSpeedSmoothed, lookSpeed, 1.0f - rotationSpeedSmoothing );
        previousLook = pCam->fwd;
    }

    bool isActing( Entity* pPlayer ) {

        static uint8_t old_weaponAim;
        static uint16_t old_clipAmmo;
        static uint32_t old_weaponHandle;
        static float old_plasmaUsed, old_plasmaCharge;

        bool isThrowingGrenade = pPlayer->animId == 0xBC && pPlayer->animFrame < 18;

        auto weaponHandle = pPlayer->childHandle;
        auto weaponRec = getEntityRecord( weaponHandle );
        auto weapon = getEntityPointer( weaponRec );

        // Check if firing.
        if ( weapon ) {
            auto clipAmmo = weapon->clipAmmo;
            auto plasmaUsed = weapon->plasmaUsed;
            if ( weaponHandle != old_weaponHandle ) {
                unpauseForNMilis( unpauseWeaponSwapMilis );
            } else if ( plasmaUsed > old_plasmaUsed || clipAmmo < old_clipAmmo ) {
                unpauseForNMilis( unpauseAfterFireMilis );
            }
            old_clipAmmo = clipAmmo;
            old_plasmaUsed = plasmaUsed;
            old_weaponHandle = weaponHandle;
        }

        auto weaponAnim = pPlayer->weaponAnim;
        if ( weaponAnim != old_weaponAim ) {
            if ( isReloading( pPlayer ) )
                unpauseForNMilis( unpauseReloadMilis );
            if ( isDoingMelee( pPlayer ) )
                unpauseForNMilis( unpauseMeleeMilis );
        }
        old_weaponAim = weaponAnim;

        uint64_t now = GetTickCount64();
        bool isActing = playerIsActingUntil > now || isThrowingGrenade;

        if ( pPlayer->parentHandle != NULL_HANDLE )
            isActing |=
            GetAsyncKeyState( 'W' ) || GetAsyncKeyState( 'A' ) ||
            GetAsyncKeyState( 'S' ) || GetAsyncKeyState( 'D' ) ||
            GetAsyncKeyState( VK_LBUTTON ); // TODO: Replace with checks that respect keybindings.

        // Check if charging plasma bolt.
        if ( weapon ) {
            float plasmaCharge = weapon->plasmaCharge;
            if ( plasmaCharge > old_plasmaCharge )
                isActing = true;
            old_plasmaCharge = plasmaCharge;
        }

        return isActing;

    }

    void update() {
        EntityRecord* playerRec = getEntityRecord( getPlayerHandle() );
        Entity* pPlayer = getEntityPointer( playerRec );
        if ( !pPlayer )
            return;

        float targetActivityLevel = isActing( pPlayer ) ? 1.0f : 0.0f;

        activityLevel = lerp( activityLevel, targetActivityLevel, activityDecayRate );

        float playerSpeed = Vec::length( pPlayer->vel );
        float speedLevel = playerSpeed / walkingSpeed;

        updateLookSpeed();
        float lookSpeedLevel = smoothstep( 0.0f, 1.0f, lookSpeedSmoothed * rotationActivityCoefficient ) * maxTimescaleDueToTurning;

        float netLevel = activityLevel + minTimescale;
        if ( pPlayer->parentHandle == NULL_HANDLE ) {
            netLevel += speedLevel + lookSpeedLevel;
        } else {
            auto vehicle = getEntityPointer( pPlayer->parentHandle );
            if ( !vehicle->fromResourcePath( "vehicles\\warthog\\warthog" ) )
                netLevel += lookSpeedLevel;
        }

        if ( isRidingTransport( pPlayer ) )
            timescale = 1.0f;
        else
            timescale = smoothstep( 0.0f, 1.0f, netLevel );
    }

}