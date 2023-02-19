#pragma once

#include "../../pch.h"

namespace Overlay {
    void cleanup();
    void onDllThreadUpdate();
    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );
}