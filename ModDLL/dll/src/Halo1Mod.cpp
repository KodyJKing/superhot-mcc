#include "headers/Halo1Mod.h"

#include "headers/Halo1.h"
#include "headers/Tracers.h"
#include "headers/Overlay.h"
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

    void onRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( mtx.try_lock() ) {

            if ( Halo1::isGameLoaded() ) {
                static std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>( pDevice, 4096 );
                Overlay::render( renderer.get(), pCtx, pDevice, pSwapChain );
                Tracers::render( renderer.get(), TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );
                TimeHack::onGameThreadUpdate();
            }

            mtx.unlock();
        }
    }

    void onDllThreadUpdate() {
        const std::lock_guard<std::mutex> lock( mtx );

        if ( Halo1::isGameLoaded() ) {
            TimeHack::onDllThreadUpdate();
            Overlay::onDllThreadUpdate();
        }
    }

}