#include "headers/TimeScale.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;
using namespace MathUtils;

namespace TimeScale {

    const float minTimescale = 0.05f;
    const float walkingSpeed = 0.07f;
    const float activityDecayRate = 0.05f;

    const float rotationActivityCoefficient = 100.0f;
    const float rotationSpeedSmoothing = 0.75f;
    const float maxTimescaleDueToTurning = 0.25f;

    const DWORD unpauseAfterFireMilis = 100;
    const DWORD unpauseReloadMilis = 750;
    const DWORD unpauseMeleeMilis = 250;
    const DWORD unpauseWeaponSwapMilis = 250;

    float activityLevel, timescale;
    uint64_t playerIsActingUntil = 0;
    Vec3 previousLook;
    float lookSpeed, lookSpeedSmoothed;

    void unpauseForNMilis( uint64_t milis ) { playerIsActingUntil = GetTickCount() + milis; }

    void updateLookSpeed() {
        auto pCam = getPlayerCameraPointer();
        float sinRot = Vec::length( Vec::cross( previousLook, pCam->fwd ) );
        lookSpeed = asinf( sinRot );
        lookSpeedSmoothed = lerp( lookSpeedSmoothed, lookSpeed, 1.0f - rotationSpeedSmoothing );
        previousLook = pCam->fwd;
    }

    uint8_t old_weaponAim;
    uint16_t old_clipAmmo;
    uint32_t old_weaponHandle;
    float old_plasmaUsed, old_plasmaCharge;
    bool isActing( Entity* pPlayer ) {

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

        if ( pPlayer->vehicleHandle != NULL_HANDLE )
            isActing |=
            GetAsyncKeyState( 'W' ) || GetAsyncKeyState( 'A' ) ||
            GetAsyncKeyState( 'S' ) || GetAsyncKeyState( 'D' ) ||
            GetAsyncKeyState( VK_LBUTTON ); // TODO: Replace with more robust checks for vehicle inputs.

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

        float netLevel = lookSpeedLevel + activityLevel + minTimescale;
        if ( pPlayer->vehicleHandle == NULL_HANDLE )
            netLevel += speedLevel;

        timescale = smoothstep( 0.0f, 1.0f, netLevel );
    }

}