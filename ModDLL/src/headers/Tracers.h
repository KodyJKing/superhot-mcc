#pragma once

#include "../../pch.h"
#include "graphics/headers/Renderer.h"

namespace Tracers {
    void render( Renderer* renderer, float time, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
}
