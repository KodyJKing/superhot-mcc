#pragma once

#include "../../pch.h"
#include "../graphics/headers/Renderer.h"

namespace Halo1Mod {

    bool init();
    void cleanup();
    void onRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
    void onDllThreadUpdate();

}