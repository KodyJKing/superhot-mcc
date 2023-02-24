#include "headers/TimeScale.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;
using namespace MathUtils;

namespace TimeScale {

    const float walkingSpeed = 0.07f;
    const float timescaleDeadzone = 0.05f;
    const float rotationActivityCoefficient = 100.0f;
    const DWORD unpauseAfterFireMilis = 100;
    const DWORD unpauseReloadMilis = 250;
    const float activityDecayRate = 0.05f;
    const float maxTimescaleDueToTurning = 0.5f;

    float activityLevel, timescale;
    uint64_t playerIsActingUntil = 0;
    Vec3 previousLook;
    float lookSpeed, lookSpeedSmoothed;

    void updateLookSpeed() {
        auto pCam = getPlayerCameraPointer();
        float sinRot = Vec::length( Vec::cross( previousLook, pCam->fwd ) );
        lookSpeed = asinf( sinRot );
        lookSpeedSmoothed = lerp( lookSpeedSmoothed, lookSpeed, 0.5f );
        previousLook = pCam->fwd;
    }

    uint16_t previousClipAmmo;
    uint32_t previousWeaponHandle;
    float previousPlasmaUsed;
    float previousPlasmaCharge;
    bool isActing( Entity* pPlayer ) {

        bool isThrowingGrenade = pPlayer->animId == 0xBC && pPlayer->animFrame < 18;

        auto weaponHandle = pPlayer->childHandle;
        auto weaponRec = getEntityRecord( weaponHandle );
        auto weapon = getEntityPointer( weaponRec );

        // Check if firing.
        if ( weapon ) {
            auto clipAmmo = weapon->clipAmmo;
            auto plasmaUsed = weapon->plasmaUsed;
            if ( weaponHandle == previousWeaponHandle ) {
                if ( plasmaUsed > previousPlasmaUsed || clipAmmo < previousClipAmmo ) {
                    // std::cout << "Player fired.\n";
                    playerIsActingUntil = GetTickCount() + unpauseAfterFireMilis;
                }
            }
            previousClipAmmo = clipAmmo;
            previousPlasmaUsed = plasmaUsed;
            previousWeaponHandle = weaponHandle;
        }

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
            if ( plasmaCharge > previousPlasmaCharge )
                isActing = true;
            previousPlasmaCharge = plasmaCharge;
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

        float netLevel = lookSpeedLevel + activityLevel;
        if ( pPlayer->vehicleHandle == NULL_HANDLE )
            netLevel += speedLevel;

        timescale = smoothstep( 0.0f, 1.0f, netLevel );
    }


    void init() {

    }

}