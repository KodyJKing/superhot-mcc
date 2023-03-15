#pragma once

#include "../../pch.h"
#include "../graphics/headers/Renderer.h"

namespace Halo1Mod {

    void init();
    void cleanup();
    void onRender( Renderer* renderer, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
    void onDllThreadUpdate();

}