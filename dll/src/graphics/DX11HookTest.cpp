#include "./headers/DX11HookTest.h"
#include "./headers/DX11Hook.h"
#include "../utils/headers/Vec3.h"

const char* shaderSource =
"\nfloat4 VS(float4 inPos : POSITION) : SV_POSITION {"
"\n    return inPos;"
"\n}"
"\nfloat4 PS(): SV_TARGET {"
"\n    return float4(1.0f, 1.0f, 1.0f, 1.0f);"
"\n}"
"\n"
;

namespace DX11HookTest {

    // Globals //
    ID3D11Buffer* triangleVertBuffer;
    ID3D11VertexShader* VS;
    ID3D11PixelShader* PS;
    ID3D10Blob* VS_Buffer;
    ID3D10Blob* PS_Buffer;
    ID3D11InputLayout* vertLayout;
    bool hasDoneDeviceInit;

    struct Vertex {
        Vec3 pos;
    };

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    void deviceInit( ID3D11Device* pDevice ) {

        hasDoneDeviceInit = true;

        // Compile Shaders from shader file
        ID3D10Blob* errorBlob;
        HRESULT hr;
        const size_t shaderSize = std::strlen( shaderSource );

        hr = D3DX11CompileFromMemory( shaderSource, shaderSize, "Effects.fx", 0, 0, "VS", "vs_4_0",
            0, 0, 0, &VS_Buffer, &errorBlob, 0 );
        if ( FAILED( hr ) ) {
            std::cout << "Error compiling vertex shader." << std::endl;
            if ( errorBlob ) {
                std::cout << (char*) errorBlob->GetBufferPointer() << std::endl;
                errorBlob->Release();
            }
            return;
        }
        hr = D3DX11CompileFromMemory( shaderSource, shaderSize, "Effects.fx", 0, 0, "PS", "ps_4_0",
            0, 0, 0, &PS_Buffer, &errorBlob, 0 );
        if ( FAILED( hr ) ) {
            std::cout << "Error compiling pixel shader." << std::endl;
            if ( errorBlob ) {
                std::cout << (char*) errorBlob->GetBufferPointer() << std::endl;
                errorBlob->Release();
            }
            return;
        }

        // Create the Shader Objects
        hr = pDevice->CreateVertexShader( VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS );
        if ( FAILED( hr ) ) {
            std::cout << "Could not create vertex shader" << std::endl;
            return;
        }
        hr = pDevice->CreatePixelShader( PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS );
        if ( FAILED( hr ) ) {
            std::cout << "Could not create pixel shader" << std::endl;
            return;
        }

        // Create the vertex buffer
        Vertex v[] = {
            { {  0.0f,  0.5f, 0.5f } },
            { {  0.0f, -0.5f, 0.5f } },
            { { -0.5f, -0.5f, 0.5f } }
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

        ID3D11DeviceContext* ctx;
        pDevice->GetImmediateContext( &ctx );

        // Set the vertex buffer
        UINT stride = sizeof( Vertex );
        UINT offset = 0;
        ctx->IASetVertexBuffers( 0, 1, &triangleVertBuffer, &stride, &offset );

        // Create the Input Layout
        pDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), VS_Buffer->GetBufferPointer(),
            VS_Buffer->GetBufferSize(), &vertLayout );

    }

    void onPresent( IDXGISwapChain* pSwapChain, ID3D11Device* pDevice ) {
        // std::cout << "Swap chain: " << (uint64_t) pSwapChain << std::endl;
        // std::cout << "Device: " << (uint64_t) pDevice << std::endl;

        if ( !hasDoneDeviceInit )
            deviceInit( pDevice );

        // Get device context
        ID3D11DeviceContext* ctx;
        pDevice->GetImmediateContext( &ctx );

        // Set Vertex and Pixel Shaders
        ctx->VSSetShader( VS, 0, 0 );
        ctx->PSSetShader( PS, 0, 0 );

        // Set the vertex buffer
        UINT stride = sizeof( Vertex );
        UINT offset = 0;
        ctx->IASetVertexBuffers( 0, 1, &triangleVertBuffer, &stride, &offset );

        // Set the Input Layout
        ctx->IASetInputLayout( vertLayout );

        // Set Primitive Topology
        ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        // // Create and set depth-stencil-state.
        // UINT ref;
        // ID3D11DepthStencilState* dss;
        // ID3D11DepthStencilState* myDss;
        // D3D11_DEPTH_STENCIL_DESC desc;
        // ZeroMemory( &desc, sizeof( desc ) );
        // desc.DepthEnable = false;
        // desc.StencilEnable = false;
        // desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        // pDevice->CreateDepthStencilState( &desc, &myDss );
        // ctx->OMGetDepthStencilState( &dss, &ref );
        // ctx->OMSetDepthStencilState( myDss, ref );

        // Draw the triangle
        ctx->Draw( 3, 0 );

        // // Restore old depth-stencil-state.
        // ctx->OMSetDepthStencilState( dss, ref );
        // // Actually, don't release, D3D11 interns these anyway.
        // // myDss->Release();
    }

    void init() {
        DX11Hook::addOnPresentCallback( onPresent );
    }

    void cleanup() {
        triangleVertBuffer->Release();
        VS->Release();
        PS->Release();
        VS_Buffer->Release();
        PS_Buffer->Release();
        vertLayout->Release();
    }

}