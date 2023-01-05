#include "../pch.h"
#include "./headers/DX11Hook.h"
#include "./headers/Hook.h"

const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

using Hook::JumpHook;

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

    // We're hooking the begining of the Present function so we can use the same positional arguments.
    void __stdcall onPresentCalled(IDXGISwapChain* pSwapChain) {
        std::cout << "Swap chain: " << (uint64_t) pSwapChain << std::endl;

        ID3D11Device* pDevice;
        if ( FAILED( pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice) ) ) {
            std::cout << "Could not get device." << std::endl;
            return;
        }

        std::cout << "Device: " << (uint64_t) pDevice << std::endl;
    }

    void addPresentHook() {
        IDXGISwapChain* pSwapChain;
        ID3D11Device* pDevice;
        D3D_FEATURE_LEVEL featureLevel;
        ID3D11DeviceContext* pDeviceContext;
        createDummy(&pSwapChain, &pDevice, &featureLevel, &pDeviceContext);

        UINT_PTR* vtable = *( (UINT_PTR**) pSwapChain);
        UINT_PTR presentMethodAddress = vtable[8];
        std::cout << "Present function address: " << presentMethodAddress << "\n\n";

        auto hook = Hook::ezCreateJumpHook(
            "Present",
            presentMethodAddress, 5,
            (UINT_PTR) onPresentCalled,
            HK_STOLEN_AFTER|HK_PUSH_STATE
        );
        hook->fixStolenOffset(1);
        hook->protectTrampoline();
        Hook::removeBeforeClosing(hook);
        hook->hook();

        pSwapChain->Release();
        pDevice->Release();
        pDeviceContext->Release();
    }
    
    // void callPresent() {
    //     IDXGISwapChain* pSwapChain{};
    //     pSwapChain->Present(0x99, 0x42);
    // }
    
    void init() {
        // std::cout << "Call present func at: " << (UINT_PTR) callPresent << std::endl;
        addPresentHook();
    }

}
