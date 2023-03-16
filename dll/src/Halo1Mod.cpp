#include "headers/Halo1Mod.h"

#include "headers/Halo1.h"
#include "headers/Tracers.h"
#include "headers/Overlay.h"
#include "timehack/headers/TimeHack.h"
#include "graphics/headers/DX11Hook.h"

namespace Halo1Mod {

    uint64_t dllBase = 0;
    bool hasInit = false;
    std::mutex mtx;

    void init() {
        mtx.lock();

        dllBase = (uint64_t) GetModuleHandleA( "halo1.dll" );
        if ( !dllBase ) return;
        Halo1::init( dllBase );
        TimeHack::init( dllBase );
        DX11Hook::addOnPresentCallback( onRender );
        hasInit = true;

        mtx.unlock();
    }

    void cleanup() {
        mtx.lock();

        std::cout << "Cleaning up Halo CE mod.\n";
        DX11Hook::removeOnPresentCallback( onRender );
        TimeHack::cleanup();

        mtx.unlock();
    }

    void onRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( mtx.try_lock() ) {

            if ( hasInit && Halo1::isGameLoaded() ) {
                static std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>( pDevice, 4096 );
                Overlay::render( renderer.get(), pCtx, pDevice, pSwapChain );
                Tracers::render( renderer.get(), TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );
                TimeHack::onGameThreadUpdate();
            }

            mtx.unlock();
        }
    }

    void onDllThreadUpdate() {
        mtx.lock();

        if ( hasInit && Halo1::isGameLoaded() ) {
            TimeHack::onDllThreadUpdate();
            Overlay::onDllThreadUpdate();
        }

        mtx.unlock();
    }

}