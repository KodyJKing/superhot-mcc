#include "headers/Halo1Mod.h"

#include "headers/Halo1.h"
#include "headers/HaloMCC.h"
#include "headers/Tracers.h"
#include "headers/Overlay.h"
#include "utils/headers/Vec.h"
#include "utils/headers/common.h"
#include "timehack/headers/TimeHack.h"
#include "graphics/headers/DX11Hook.h"

namespace Halo1Mod {

    uint64_t dllBase = 0;
    std::mutex mtx;

    bool init() {
        const std::lock_guard<std::mutex> lock( mtx );

        dllBase = (uint64_t) GetModuleHandleA( "halo1.dll" );
        if ( !dllBase )
            return false;

        Halo1::init( dllBase );

        if ( !TimeHack::init( dllBase ) )
            return false;

        DX11Hook::addOnPresentCallback( onRender );

        return true;
    }

    void cleanup() {
        const std::lock_guard<std::mutex> lock( mtx );

        std::cout << "Cleaning up Halo CE mod.\n";
        DX11Hook::removeOnPresentCallback( onRender );
        TimeHack::cleanup();
    }

    void teleportToCrosshair() {
        Halo1::RaycastResult result;
        bool didHit = Halo1::raycastPlayerSightLine( result );
        if ( !didHit )
            return;

        auto cam = Halo1::getPlayerCameraPointer();
        auto playerRec = Halo1::getPlayerRecord();
        if ( !playerRec )
            return;

        auto player = playerRec->entity();
        player->pos = Vec::addScaled( result.pos, cam->fwd, -1.0f );
        player->vel = Vec::addScaled( player->vel, cam->fwd, 0.15f );
    }

    void onRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( mtx.try_lock() ) {

            if ( Halo1::isGameLoaded() ) {
                auto renderer = Renderer::getDefault( pDevice );

                #ifdef _DEBUG
                Overlay::render( renderer, pCtx, pDevice, pSwapChain );
                #endif

                Tracers::render( renderer, TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );

                TimeHack::onGameThreadUpdate();

                #ifdef _DEBUG
                if ( HaloMCC::isInForeground() ) {
                    if ( keypressed( 'T' ) )
                        teleportToCrosshair();
                    if ( keypressed( 'M' ) ) {
                        auto name = Halo1::getMapName();
                        if ( name )
                            std::cout << name << "\n";
                    }
                }
                #endif
            }

            mtx.unlock();
        }
    }

    void onDllThreadUpdate() {
        const std::lock_guard<std::mutex> lock( mtx );

        if ( Halo1::isGameLoaded() ) {
            TimeHack::onDllThreadUpdate();
            #ifdef _DEBUG
            Overlay::onDllThreadUpdate();
            #endif
        }
    }

}