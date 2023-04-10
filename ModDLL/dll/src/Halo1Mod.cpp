#include "headers/Halo1Mod.h"

#include "headers/Halo1.h"
#include "headers/HaloMCC.h"
#include "headers/Tracers.h"
#include "headers/Overlay.h"
#include "utils/headers/Vec.h"
#include "utils/headers/Hook.h"
#include "utils/headers/common.h"
#include "utils/headers/CrashReporting.h"
#include "graphics/headers/DX11Utils.h"
#include "timehack/headers/TimeHack.h"
#include "graphics/headers/DX11Hook.h"

using namespace Hook;
using std::make_unique;
using std::unique_ptr;

void testCrash() {
    Vec3* badPtr = nullptr;
    std::cout << badPtr->x;
}

static XMMATRIX anniversaryCamMatrix;

void printMat( XMMATRIX* pMat ) {
    float* fmat = (float*) pMat;
    for ( int i = 0; i < 4; i++ ) {
        for ( int j = 0; j < 4; j++ ) {
            std::cout << std::setw( 12 ) << std::fixed << std::setfill( ' ' ) << fmat[j + i * 4];
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

extern "C" {
    void setViewProjMatrixHook();
    uint64_t setViewProjMatrixHook_return;
    void copyViewProjMatrix( XMMATRIX* pViewProjMatrix ) {
        anniversaryCamMatrix = *pViewProjMatrix;

        auto pCam = Halo1::getPlayerCameraPointer();
        anniversaryCamMatrix = XMMatrixTranspose( *pViewProjMatrix ); // *pViewProjMatrix; // cameraMatrix( pCam->pos, *pViewProjMatrix );

        static int callCount = 0;
        if ( callCount++ % 100 == 0 ) {
            printMat( &anniversaryCamMatrix );

            XMMATRIX transform;
            auto screenDimensions = HaloMCC::getWindowSize();
            auto oldCamPos = pCam->pos;
            pCam->pos = { 0.0f, 0.0f, 0.0f };
            Halo1::getCameraMatrix( screenDimensions.x, screenDimensions.y, transform );
            pCam->pos = oldCamPos;
            printMat( &transform );
        }
    }
}

namespace Halo1Mod {

    static std::vector<HookPointer> hooks;
    uint64_t dllBase = 0;
    std::mutex mtx;

    bool init() {
        const std::lock_guard<std::mutex> lock( mtx );

        std::cout << "Initializing Halo 1 mod.\n";

        dllBase = (uint64_t) GetModuleHandleA( "halo1.dll" );
        if ( !dllBase ) {
            std::cout << "Failed. Halo1.dll not found.\n";
            return false;
        }

        Halo1::init( dllBase );

        if ( !TimeHack::init( dllBase ) ) {
            std::cout << "Failed. Couldn't initialize time hack.\n";
            return false;
        }

        DX11Hook::addOnPresentCallback( onRender );

        hooks.clear();
        hooks.emplace_back( make_unique<JumpHook>(
            "Copy ViewProjection Matrix (anniversary mode)",
            dllBase + 0x22FCC0U, 11,
            (UINT_PTR) setViewProjMatrixHook,
            setViewProjMatrixHook_return
        ) );

        std::cout << "Halo 1 mod initialized.\n";

        return true;
    }

    void cleanup() {
        const std::lock_guard<std::mutex> lock( mtx );

        std::cout << "Cleaning up Halo CE mod.\n";
        hooks.clear();
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
        CrashReporting::initializeForCurrentThread();

        if ( mtx.try_lock() ) {

            if ( Halo1::isGameLoaded() ) {
                auto renderer = Renderer::getDefault( pDevice );

                #ifdef _DEBUG
                Overlay::render( renderer, pCtx, pDevice, pSwapChain );
                #endif

                Tracers::render( renderer, TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );

                TimeHack::onGameThreadUpdate();

                // if ( keypressed( VK_NUMPAD8 ) )
                //     testCrash();

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

        // if ( keypressed( VK_NUMPAD7 ) )
        //     testCrash();

        if ( Halo1::isGameLoaded() ) {
            TimeHack::onDllThreadUpdate();
            #ifdef _DEBUG
            Overlay::onDllThreadUpdate();
            #endif
        }
    }

}