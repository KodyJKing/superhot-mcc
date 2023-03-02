#pragma once

#include "../../pch.h"
#include "../graphics/headers/Renderer.h"

namespace Overlay {
    void onDllThreadUpdate();
    void render( Renderer* renderer, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
}