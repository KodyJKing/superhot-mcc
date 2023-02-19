#pragma once

#include "../../pch.h"


namespace Overlay {
    void cleanup();
    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
}