#include "../utils/headers/common.h"
#include "./headers/DX11MethodOffsets.h"
#include "./headers/DX11Hook.h"
#include "../headers/Hook.h"
#include "../../pch.h"

using DX11Hook::PresentCallback;
using std::make_unique;
using Hook::JumpHook;

extern "C" {
    uint64_t presentHook_jmp;
    void presentHook();
    void __stdcall onPresentCalled(
        IDXGISwapChain* pSwapChain,
        UINT SyncInterval,
        UINT Flags
    );

    uint64_t resizeBuffersHook_jmp;
    void resizeBuffersHook();
    void onResizeBuffers(
        IDXGISwapChain* pSwapChain,
        UINT           BufferCount,
        UINT           Width,
        UINT           Height,
        DXGI_FORMAT    NewFormat,
        UINT           SwapChainFlags
    );

    uint64_t setRenderTargetsHook_return;
    uint64_t setRenderTargetsHook_depthStencilView;
    void setRenderTargetsHook();
}

static std::vector<PresentCallback> onPresentCallbacks;
static std::mutex onPresentCallbacks_mutex;
static bool hasDoneDeviceInit;
static bool deviceInitFailed;
static ID3D11RenderTargetView* renderTargetView;

HRESULT createDummy(
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppDeviceContext,
    HWND hwnd
);

bool initDevice( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );

// We're hooking the begining of the Present function so we can use the same positional arguments.
void __stdcall onPresentCalled(
    IDXGISwapChain* pSwapChain,
    UINT SyncInterval,
    UINT Flags
) {
    ID3D11Device* pDevice;
    if ( FAILED( pSwapChain->GetDevice( __uuidof( ID3D11Device ), (void**) &pDevice ) ) ) {
        std::cout << "Could not get device." << std::endl;
        return;
    }

    if ( !hasDoneDeviceInit )
        initDevice( pDevice, pSwapChain );

    auto depthStencilView = (ID3D11DepthStencilView*) setRenderTargetsHook_depthStencilView;

    ID3D11DeviceContext* pCtx;
    pDevice->GetImmediateContext( &pCtx );
    pCtx->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

    if ( onPresentCallbacks_mutex.try_lock() ) {
        for ( PresentCallback cb : onPresentCallbacks )
            cb( pCtx, pDevice, pSwapChain );
        onPresentCallbacks_mutex.unlock();
    }

    setRenderTargetsHook_depthStencilView = 0;
}

void onResizeBuffers(
    IDXGISwapChain* pSwapChain,
    UINT           BufferCount,
    UINT           Width,
    UINT           Height,
    DXGI_FORMAT    NewFormat,
    UINT           SwapChainFlags
) {
    /* We need to release renderTargetView before ResizeBuffers runs because
    it keeps a reference to the swap chain's back buffers alive,
    which would cause a crash. */
    std::cout << "Releasing renderTargetView for buffer resize.\n";
    safeRelease( renderTargetView );
    hasDoneDeviceInit = false;
    deviceInitFailed = false;
}

namespace DX11Hook {

    static std::vector<HookPointer> hooks;

    /// @brief  Setup a hook to call custom rendering functions added through DX11Hook::addOnPresentCallback.
    /// @param hwnd The window you will be rendering.
    /// @param pSwapChainActual [Optional] A pointer to an existing swap chain to use for virtual table hooking. If this is null, a dummy swap chain is created.
    void hook( HWND hwnd, IDXGISwapChain* pSwapChainActual ) {
        IDXGISwapChain* pSwapChain;
        ID3D11Device* pDevice;
        D3D_FEATURE_LEVEL featureLevel;
        ID3D11DeviceContext* pDeviceContext;

        if ( pSwapChainActual ) {
            pSwapChain = pSwapChainActual;
            auto hr = pSwapChainActual->GetDevice( __uuidof( ID3D11Device ), (void**) &pDevice );
            if ( FAILED( hr ) ) return;
            pDevice->GetImmediateContext( &pDeviceContext );
        } else {
            createDummy( &pSwapChain, &pDevice, &featureLevel, &pDeviceContext, hwnd );
        }

        UINT_PTR* swapChainVTable = *( (UINT_PTR**) pSwapChain );
        UINT_PTR* deviceContextVTable = *( (UINT_PTR**) pDeviceContext );

        hooks.clear();

        /* We're hooking sites already hooked by Steam's overlay, so we don't need
        to use return jumps. It is enough to execute the stolen jump. Steam will
        jump back into DirectX's code for us. */

        uint64_t presentHook_start = swapChainVTable[MO_IDXGISwapChain::Present];
        presentHook_jmp = Hook::getJumpDestination( presentHook_start );
        hooks.emplace_back( make_unique<JumpHook>(
            "Present",
            presentHook_start, 5,
            (UINT_PTR) presentHook
        ) );

        uint64_t resizeBuffersHook_start = swapChainVTable[MO_IDXGISwapChain::ResizeBuffers];
        resizeBuffersHook_jmp = Hook::getJumpDestination( resizeBuffersHook_start );
        hooks.emplace_back( make_unique<JumpHook>(
            "ResizeBuffers",
            resizeBuffersHook_start, 5,
            (UINT_PTR) resizeBuffersHook
        ) );

        hooks.emplace_back( make_unique<JumpHook>(
            "SetRenderTargets",
            deviceContextVTable[MO_ID3D11DeviceContext::OMSetRenderTargets], 5,
            (UINT_PTR) setRenderTargetsHook,
            setRenderTargetsHook_return
        ) );

        // std::cout << "\n";
        // std::cout << "SwapChain.Present address: " << std::uppercase << std::hex << swapChainVTable[MO_IDXGISwapChain::Present] << "\n";
        // std::cout << "SwapChain.ResizeBuffers address: " << std::uppercase << std::hex << swapChainVTable[MO_IDXGISwapChain::ResizeBuffers] << "\n";
        // std::cout << "Context.Draw address: " << std::uppercase << std::hex << deviceContextVTable[MO_ID3D11DeviceContext::Draw] << "\n";
        // std::cout << "Context.DrawIndexed address: " << std::uppercase << std::hex <<  deviceContextVTable[MO_ID3D11DeviceContext::DrawIndexed] << "\n";
        // std::cout << "Context.OMSetRenderTargets address: " << std::uppercase << std::hex << deviceContextVTable[MO_ID3D11DeviceContext::OMSetRenderTargets] << "\n";
        // std::cout << "\n";

        if ( !pSwapChainActual ) {
            // The we created dummies and need to clean them up.
            pSwapChain->Release();
            pDevice->Release();
            pDeviceContext->Release();
        }

    }


    /// @brief Uninstall rendering hooks.
    void cleanup() {
        hooks.clear();
        safeRelease( renderTargetView );
    }

    /// @brief Adds a render function to run every frame.
    /// @param cb Render function pointer.
    void addOnPresentCallback( PresentCallback cb ) {
        onPresentCallbacks_mutex.lock();
        onPresentCallbacks.emplace_back( cb );
        onPresentCallbacks_mutex.unlock();
    }

}

HRESULT createDummy(
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppDeviceContext,
    HWND hwnd
) {

    static const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    return D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        featureLevels,
        ARRAYSIZE( featureLevels ),
        D3D11_SDK_VERSION,
        &sd,
        ppSwapChain,
        ppDevice,
        pFeatureLevel,
        ppDeviceContext
    );
}

bool initDevice( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
    if ( deviceInitFailed )
        return false;
    if ( hasDoneDeviceInit )
        return true;

    ID3D11Texture2D* pBackBuffer;
    auto hr = pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) &pBackBuffer );
    if ( FAILED( hr ) ) {
        std::cout << "Coud not get buffer.\n";
        deviceInitFailed = true;
        return false;

    }

    hr = pDevice->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView );
    if ( FAILED( hr ) ) {
        std::cout << "Coud not create render target view.\n";
        deviceInitFailed = true;
        pBackBuffer->Release();
        return false;
    }

    pBackBuffer->Release();
    hasDoneDeviceInit = true;
    return true;
}