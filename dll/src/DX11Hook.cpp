#include "../pch.h"
#include "./headers/DX11Hook.h"

const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

namespace DX11Hook {

    void createDummy(
        IDXGISwapChain** ppSwapChain, 
        ID3D11Device** ppDevice, 
        D3D_FEATURE_LEVEL* pFeatureLevel,
        ID3D11DeviceContext** ppDeviceContext
    ) {

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = GetForegroundWindow();
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            0,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &sd,
            ppSwapChain,
            ppDevice,
            pFeatureLevel,
            ppDeviceContext
        );
    }

    void init() {

        IDXGISwapChain* pSwapChain;
        ID3D11Device* pDevice;
        D3D_FEATURE_LEVEL featureLevel;
        ID3D11DeviceContext* pDeviceContext;

        createDummy(&pSwapChain, &pDevice, &featureLevel, &pDeviceContext);

        std::cout << "Created dummy device and swapchain with no errors!" << std::endl;

        pSwapChain->Release();
        pDevice->Release();
        pDeviceContext->Release();
        
    }

}
