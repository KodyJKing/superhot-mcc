#include "headers/Halo1Mod.h"

#include "headers/Halo1.h"
#include "headers/Tracers.h"
#include "headers/Overlay.h"
#include "timehack/headers/TimeHack.h"

namespace Halo1Mod {

    uint64_t dllBase = 0;

    void init() {
        dllBase = (uint64_t) GetModuleHandleA( "halo1.dll" );
        if ( !dllBase )
            return;

        std::cout << "halo1.dll located at: ";
        std::cout << std::uppercase << std::hex << dllBase;
        std::cout << "\n\n";

        Halo1::init( dllBase );
        TimeHack::init( dllBase );

    }

    void cleanup() {
        std::cout << "Cleaning up Halo CE mod.\n";

        TimeHack::cleanup();
    }

    void onRender( Renderer* renderer, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( Halo1::isGameLoaded() ) {
            Overlay::render( renderer, pCtx, pDevice, pSwapChain );
            Tracers::render( renderer, TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );
            TimeHack::onGameThreadUpdate();
        }
    }

    void onDllThreadUpdate() {
        if ( Halo1::isGameLoaded() ) {
            TimeHack::onDllThreadUpdate();
            Overlay::onDllThreadUpdate();
        }
    }

}