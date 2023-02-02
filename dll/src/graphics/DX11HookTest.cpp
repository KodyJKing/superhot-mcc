#include "./headers/DX11HookTest.h"
#include "./headers/DX11Hook.h"
#include "./headers/DX11Utils.h"
#include "../utils/headers/Vec3.h"
#include "../utils/headers/common.h"

namespace DX11HookTest {

    const char* shaderSource = R"(
        float4 VS(float4 inPos : POSITION) : SV_POSITION {
            return inPos;
        }
        float4 PS(float4 pos : SV_POSITION): SV_TARGET {
            return float4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    )";

    // Globals //
    ID3D11Buffer* triangleVertBuffer;
    ID3D11VertexShader* VS;
    ID3D11PixelShader* PS;
    ID3D11InputLayout* vertLayout;
    ID3D11BlendState* blendState;
    ID3D11DepthStencilState* depthStencilState;

    struct Vertex {
        float x, y, z;
    };

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    bool hasDoneDeviceInit;
    bool deviceInitFailed;
    void _deviceInit( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        std::cout << "About to compile shaders" << std::endl;
        VS = compileVertexShader( pDevice, shaderSource, "VS", "vs_4_0", layout, ARRAYSIZE( layout ), &vertLayout );
        PS = compilePixelShader( pDevice, shaderSource, "PS", "ps_4_0" );

        // Create the vertex buffer
        Vertex v[] = {
            {  0.0f,  0.5f, 0.99f },
            {  0.5f, -0.5f, 0.99f },
            { -0.5f, -0.5f, 0.99f }
        };

        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory( &vertexBufferDesc, sizeof( vertexBufferDesc ) );
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof( Vertex ) * ARRAYSIZE( v );
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA vertexBufferData;
        ZeroMemory( &vertexBufferData, sizeof( vertexBufferData ) );
        vertexBufferData.pSysMem = v;

        pDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &triangleVertBuffer );

        // Create blend state;
        D3D11_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget->BlendEnable = true;
        blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        throwIfFail( pDevice->CreateBlendState( &blendDesc, &blendState ) );

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        auto desc = depthStencilDesc;
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.StencilEnable = false;
        throwIfFail( "Creating depth-stencil state", pDevice->CreateDepthStencilState( &desc, &depthStencilState ) );
    }

    void deviceInit( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( hasDoneDeviceInit ) return;
        hasDoneDeviceInit = true;
        try {
            _deviceInit( pDevice, pSwapChain );
        } catch ( std::exception& e ) {
            std::cout << "Could not initialize device." << std::endl;
            std::cout << e.what() << std::endl;
            deviceInitFailed = true;
        }
    }

    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
        if ( !hasDoneDeviceInit ) deviceInit( pDevice, pSwapChain );
        if ( deviceInitFailed ) return;

        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc( &sd );
        fitViewportToWindow( pCtx, sd.OutputWindow );

        pCtx->VSSetShader( VS, 0, 0 );
        pCtx->PSSetShader( PS, 0, 0 );

        pCtx->OMSetBlendState( blendState, nullptr, 0xFFFFFFFF );
        pCtx->OMSetDepthStencilState( depthStencilState, 1 );

        UINT stride = sizeof( Vertex );
        UINT offset = 0;
        pCtx->IASetVertexBuffers( 0, 1, &triangleVertBuffer, &stride, &offset );

        pCtx->IASetInputLayout( vertLayout );
        pCtx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        pCtx->Draw( 3, 0 );
    }

    void init() {
        DX11Hook::addOnPresentCallback( render );
    }

    void cleanup() {
        safeRelease( triangleVertBuffer );
        safeRelease( VS );
        safeRelease( PS );
        safeRelease( vertLayout );
        safeRelease( blendState );
        safeRelease( depthStencilState );
    }

}