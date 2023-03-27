#include "./headers/DX11Hook.h"
#include "../../pch.h"
#include "../utils/headers/common.h"
#include "../utils/headers/BytePattern.h"
#include "../utils/headers/Hook.h"
#include "./headers/DX11MethodOffsets.h"

using DX11Hook::PresentCallback;
using Hook::VirtualTableHook;
using std::make_unique;

HRESULT createDummy(
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppDeviceContext,
    HWND hwnd
);

bool initDevice( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );

static std::set<PresentCallback> onPresentCallbacks;
static std::mutex onPresentCallbacks_mutex;
static bool hasDoneDeviceInit;
static bool deviceInitFailed;
static ID3D11RenderTargetView* renderTargetView;
static ID3D11DepthStencilView* firstDepthStencilView;

static HRESULT( *Present )(
    IDXGISwapChain* pSwapChain,
    UINT SyncInterval,
    UINT Flags
    );
HRESULT onPresent(
    IDXGISwapChain* pSwapChain,
    UINT SyncInterval,
    UINT Flags
) {
    ID3D11Device* pDevice;
    if ( FAILED( pSwapChain->GetDevice( __uuidof( ID3D11Device ), (void**) &pDevice ) ) ) {
        std::cout << "Could not get device." << std::endl;
    } else {
        if ( !hasDoneDeviceInit )
            initDevice( pDevice, pSwapChain );

        ID3D11DeviceContext* pCtx;
        pDevice->GetImmediateContext( &pCtx );
        pCtx->OMSetRenderTargets( 1, &renderTargetView, firstDepthStencilView );

        if ( onPresentCallbacks_mutex.try_lock() ) {
            for ( PresentCallback cb : onPresentCallbacks )
                cb( pCtx, pDevice, pSwapChain );
            onPresentCallbacks_mutex.unlock();
        }

        firstDepthStencilView = nullptr;
    }
    return Present( pSwapChain, SyncInterval, Flags );
}

static HRESULT( *ResizeBuffers )(
    IDXGISwapChain* pSwapChain,
    UINT           BufferCount,
    UINT           Width,
    UINT           Height,
    DXGI_FORMAT    NewFormat,
    UINT           SwapChainFlags
    );
HRESULT onResizeBuffers(
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
    return ResizeBuffers( pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags );
}

static void ( *SetRenderTargets )(
    ID3D11DeviceContext* pCtx,
    UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView
    );
void onSetRenderTargets(
    ID3D11DeviceContext* pCtx,
    UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView
) {
    if ( !firstDepthStencilView )
        firstDepthStencilView = pDepthStencilView;
    SetRenderTargets( pCtx, NumViews, ppRenderTargetViews, pDepthStencilView );
}

namespace DX11Hook {

    static std::vector<VHookPointer> hooks;

    /// @brief  Setup a hook to call custom rendering functions added through DX11Hook::addOnPresentCallback.
    /// @param hwnd The window you will be rendering.
    /// @param pSwapChainActual [Optional] A pointer to an existing swap chain to use for virtual table hooking. If this is null, a dummy swap chain is created.
    HRESULT hook( HWND hwnd, IDXGISwapChain* pSwapChainActual ) {
        IDXGISwapChain* pSwapChain;
        ID3D11Device* pDevice;
        D3D_FEATURE_LEVEL featureLevel;
        ID3D11DeviceContext* pDeviceContext;

        if ( pSwapChainActual ) {
            pSwapChain = pSwapChainActual;
            auto hr = pSwapChainActual->GetDevice( __uuidof( ID3D11Device ), (void**) &pDevice );
            if ( FAILED( hr ) ) {
                showAndPrintError( "Could not install DX11 hooks: Couldn't get device from swap chain." );
                return hr;
            }
            pDevice->GetImmediateContext( &pDeviceContext );
        } else {
            auto hr = createDummy( &pSwapChain, &pDevice, &featureLevel, &pDeviceContext, hwnd );
            if ( FAILED( hr ) ) {
                showAndPrintError( "Could not install DX11 hooks: Could not construct dummy device." );
                return hr;
            }
        }

        void** swapChainVTable = *( (void***) pSwapChain );
        void** deviceContextVTable = *( (void***) pDeviceContext );

        hooks.clear();

        hooks.emplace_back( make_unique<VirtualTableHook>(
            "Present", swapChainVTable, MO_IDXGISwapChain::Present, onPresent, (void**) &Present ) );
        hooks.emplace_back( make_unique<VirtualTableHook>(
            "ResizeBuffers", swapChainVTable, MO_IDXGISwapChain::ResizeBuffers, onResizeBuffers, (void**) &ResizeBuffers ) );
        hooks.emplace_back( make_unique<VirtualTableHook>(
            "SetRenderTargets", deviceContextVTable, MO_ID3D11DeviceContext::OMSetRenderTargets, onSetRenderTargets, (void**) &SetRenderTargets ) );

        // std::cout << "\n";
        // std::cout << "SwapChain.Present address: " << std::uppercase << std::hex << swapChainVTable[MO_IDXGISwapChain::Present] << "\n";
        // std::cout << "SwapChain.ResizeBuffers address: " << std::uppercase << std::hex << swapChainVTable[MO_IDXGISwapChain::ResizeBuffers] << "\n";
        // std::cout << "Context.Draw address: " << std::uppercase << std::hex << deviceContextVTable[MO_ID3D11DeviceContext::Draw] << "\n";
        // std::cout << "Context.DrawIndexed address: " << std::uppercase << std::hex << deviceContextVTable[MO_ID3D11DeviceContext::DrawIndexed] << "\n";
        // std::cout << "Context.OMSetRenderTargets address: " << std::uppercase << std::hex << deviceContextVTable[MO_ID3D11DeviceContext::OMSetRenderTargets] << "\n";
        // std::cout << "\n";

        if ( !pSwapChainActual ) {
            // We created dummies and need to clean them up.
            pSwapChain->Release();
            pDevice->Release();
            pDeviceContext->Release();
        }

        return S_OK;

    }

    /// @brief Uninstall rendering hooks.
    void cleanup() {
        const std::lock_guard<std::mutex> lock( onPresentCallbacks_mutex );
        hooks.clear();
        safeRelease( renderTargetView );
    }

    /// @brief Adds a render function to run every frame.
    /// @param cb Render function pointer.
    void addOnPresentCallback( PresentCallback cb ) {
        const std::lock_guard<std::mutex> lock( onPresentCallbacks_mutex );
        onPresentCallbacks.insert( cb );
    }

    void removeOnPresentCallback( PresentCallback cb ) {
        const std::lock_guard<std::mutex> lock( onPresentCallbacks_mutex );
        onPresentCallbacks.erase( cb );
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