#include "./headers/DX11HookTest.h"
#include "./headers/DX11Hook.h"
#include "../utils/headers/Vec3.h"
#include "../utils/headers/common.h"

void safePrintErrorMessage( ID3D10Blob* errorBlob ) {
    if ( errorBlob )
        std::cout << (char*) errorBlob->GetBufferPointer() << std::endl;
}

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
    ID3D10Blob* VS_Buffer;
    ID3D10Blob* PS_Buffer;
    ID3D10Blob* errorBlob;
    ID3D11InputLayout* vertLayout;
    ID3D11BlendState* blendState;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11RenderTargetView* renderTargetView;
    bool hasDoneDeviceInit;
    bool deviceInitFailed;

    struct Vertex {
        float x, y, z;
    };

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    void _deviceInit( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        // Compile Shaders from shader file
        HRESULT hr;
        const size_t shaderSize = std::strlen( shaderSource );

        // Create render target view
        ID3D11Texture2D* pBackBuffer;
        pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) &pBackBuffer );
        pDevice->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView );
        pBackBuffer->Release();

        // Compile shaders
        std::cout << "About to compile shaders" << std::endl;
        hr = D3DX11CompileFromMemory( shaderSource, shaderSize, 0, 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, &errorBlob, 0 );
        safePrintErrorMessage( errorBlob );
        throwIfFail( "Compiling vertex shader", hr );

        hr = D3DX11CompileFromMemory( shaderSource, shaderSize, 0, 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, &errorBlob, 0 );
        safePrintErrorMessage( errorBlob );
        throwIfFail( "Compiling pixel shader", hr );

        // Create the shader objects
        std::cout << "About to create shaders" << std::endl;
        hr = pDevice->CreateVertexShader( VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS );
        throwIfFail( "Creating vertex shader", hr );
        hr = pDevice->CreatePixelShader( PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS );
        throwIfFail( "Creating pixel shader", hr );

        // Create the vertex buffer
        Vertex v[] = {
            {  0.0f,  0.5f, 0.5f },
            {  0.5f, -0.5f, 0.5f },
            { -0.5f, -0.5f, 0.5f }
        };

        std::cout << "About to create VBO" << std::endl;
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory( &vertexBufferDesc, sizeof( vertexBufferDesc ) );
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof( Vertex ) * ARRAYSIZE( v );
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        // Create blend state;
        std::cout << "About to create render state objects" << std::endl;
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
        desc.StencilEnable = true;
        desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace = desc.FrontFace;
        throwIfFail( "Creating depth-stencil state", pDevice->CreateDepthStencilState( &desc, &depthStencilState ) );

        D3D11_SUBRESOURCE_DATA vertexBufferData;
        ZeroMemory( &vertexBufferData, sizeof( vertexBufferData ) );
        vertexBufferData.pSysMem = v;

        pDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &triangleVertBuffer );

        ID3D11DeviceContext* ctx;
        pDevice->GetImmediateContext( &ctx );

        // Create the Input Layout
        pDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), VS_Buffer->GetBufferPointer(),
            VS_Buffer->GetBufferSize(), &vertLayout );

    }

    void deviceInit( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        if ( hasDoneDeviceInit )
            return;

        hasDoneDeviceInit = true;

        try {
            _deviceInit( pDevice, pSwapChain );
        }
        catch ( std::exception& e ) {
            std::cout << "Could not initialize device." << std::endl;
            std::cout << e.what() << std::endl;
            deviceInitFailed = true;
        }

    }

    void fitViewportToWindow( ID3D11DeviceContext* ctx, HWND hwnd ) {
        RECT rect;
        GetClientRect( hwnd, &rect );

        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0;
        vp.TopLeftY = 0.0;
        vp.Width = (float) ( rect.right - rect.left );
        vp.Height = (float) ( rect.bottom - rect.top );
        vp.MinDepth = 0.0;
        vp.MaxDepth = 1.0;
        ctx->RSSetViewports( 1, &vp );
    }

    void render( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        // Get device context
        ID3D11DeviceContext* ctx;
        pDevice->GetImmediateContext( &ctx );

        if ( !hasDoneDeviceInit )
            deviceInit( pDevice, pSwapChain );
        if ( deviceInitFailed )
            return;

        ctx->OMSetRenderTargets( 1, &renderTargetView, NULL );

        // Fit window
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc( &sd );
        fitViewportToWindow( ctx, sd.OutputWindow );

        // Set Vertex and Pixel Shaders
        ctx->VSSetShader( VS, 0, 0 );
        ctx->PSSetShader( PS, 0, 0 );

        ctx->OMSetBlendState( blendState, nullptr, 0xFFFFFFFF );
        ctx->OMSetDepthStencilState( depthStencilState, 1 );

        // Set the vertex buffer
        UINT stride = sizeof( Vertex );
        UINT offset = 0;
        ctx->IASetVertexBuffers( 0, 1, &triangleVertBuffer, &stride, &offset );

        // Set the Input Layout
        ctx->IASetInputLayout( vertLayout );

        // Set Primitive Topology
        ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        // Draw the triangle
        ctx->Draw( 3, 0 );

    }

    void init() {
        DX11Hook::addOnPresentCallback( render );
    }

    void cleanup() {
        safeRelease( triangleVertBuffer );
        safeRelease( VS );
        safeRelease( PS );
        safeRelease( VS_Buffer );
        safeRelease( PS_Buffer );
        safeRelease( vertLayout );
        safeRelease( errorBlob );
        safeRelease( blendState );
        safeRelease( depthStencilState );
        safeRelease( renderTargetView );
    }

}