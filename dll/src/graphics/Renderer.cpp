//
// Thanks to Yazzn for his renderer, which I used for reference here.
// https://www.unknowncheats.me/forum/d3d-tutorials-and-source/177926-direct3d-11-renderer.html
//

#include "./headers/DX11Utils.h"
#include "./headers/Renderer.h"
#include "./headers/shader.h"

#include <DirectXPackedVector.h>

Renderer::Renderer( ID3D11Device* pDevice, uint32_t maxVertices, LPCWSTR defaultFontFamily ) {
    HRESULT hr;

    auto vboSize = maxVertices * sizeof( Vertex );

    this->pDevice = pDevice;
    this->maxVertices = maxVertices;
    pCpuVertexBuffer = (Vertex*) malloc( vboSize );
    vertexCount = 0;

    pDevice->GetImmediateContext( &pCtx );

    { // Compile shaders
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        VS = compileVertexShader( pDevice, shaderSource, "VS", "vs_4_0", layout, ARRAYSIZE( layout ), &vertLayout );
        PS = compilePixelShader( pDevice, shaderSource, "PS", "ps_4_0" );
    }

    { // Create vertex buffer
        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof( desc ) );
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = (UINT) vboSize;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        hr = pDevice->CreateBuffer( &desc, nullptr, &pVertexBuffer );

        throwIfFail( "Creating vertex buffer", hr );
    }

    { // Creating transform buffer
        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof( desc ) );
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof( XMMATRIX );
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        hr = pDevice->CreateBuffer( &desc, nullptr, &pTransformBuffer );
    }

    { // Create blend state
        D3D11_BLEND_DESC desc{};
        desc.RenderTarget->BlendEnable = true;
        desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        throwIfFail( pDevice->CreateBlendState( &desc, &blendState ) );
    }

    auto ident = XMMatrixIdentity();
    this->setTransform( &ident );

    // FW1
    this->defaultFontFamily = defaultFontFamily;
    throwIfFail( FW1CreateFactory( FW1_VERSION, &fontFactory ) );
    throwIfFail( fontFactory->CreateFontWrapper( pDevice, defaultFontFamily, &fontWrapper ) );
    throwIfFail( fontFactory->CreateTextGeometry( &textGeometry ) );
}

Renderer::~Renderer() {
    std::cout << "Cleaning up Renderer.\n";
    safeRelease( pTransformBuffer );
    safeRelease( pVertexBuffer );
    safeRelease( vertLayout );
    safeRelease( VS );
    safeRelease( PS );
    safeRelease( blendState );
    safeFree( pCpuVertexBuffer );

    // FW1
    safeRelease( fontWrapper );
    safeRelease( fontFactory );
    safeRelease( textGeometry );
}

void Renderer::setTransform( XMMATRIX* pTransform ) {
    this->flush();
    copyToBuffer( pCtx, pTransformBuffer, pTransform, sizeof( XMMATRIX ) );
}

void Renderer::setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topology ) {
    if ( topology != this->topology )
        this->flush();
    this->topology = topology;
}

void Renderer::begin() {
    pCtx->VSSetShader( VS, 0, 0 );
    pCtx->PSSetShader( PS, 0, 0 );

    pCtx->OMSetBlendState( blendState, nullptr, 0xFFFFFFFF );

    pCtx->VSSetConstantBuffers( 0, 1, &pTransformBuffer );

    pCtx->IASetInputLayout( vertLayout );

    UINT stride = sizeof( Vertex );
    UINT offset = 0;
    pCtx->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );

    // FW1
    fontWrapper->DrawString( pCtx, L"", 0.0f, 0.0f, 0.0f,
        0xff000000, FW1_RESTORESTATE | FW1_NOFLUSH );
}

void Renderer::end() {
    vertexCount = 0;

    // FW1
    textGeometry->Clear();
}

void Renderer::flush() {
    if ( vertexCount == 0 )
        return;
    size_t vertexBytes = vertexCount * sizeof( Vertex );
    copyToBuffer( pCtx, pVertexBuffer, (void*) pCpuVertexBuffer, vertexBytes );
    pCtx->IASetPrimitiveTopology( topology );
    pCtx->Draw( vertexCount, 0 );
    vertexCount = 0;

    // FW1
    fontWrapper->Flush( pCtx );
    fontWrapper->DrawGeometry( pCtx, textGeometry,
        nullptr, nullptr, FW1_RESTORESTATE );
}

void Renderer::pushVerticies( uint32_t pushCount, Vertex* pVertices ) {
    if ( vertexCount + pushCount > maxVertices )
        this->flush();
    auto pVertex = pCpuVertexBuffer + vertexCount;
    size_t pushBytes = pushCount * sizeof( Vertex );
    std::memcpy( (void*) pVertex, pVertices, pushBytes );
    vertexCount += pushCount;
}

void Renderer::drawText( Vector2 pos, LPCWSTR text, Vector4 color, uint32_t flags,
    float fontSize, LPCWSTR fontFamily ) {

    if ( !fontFamily )
        fontFamily = defaultFontFamily;

    uint32_t color32 = PackedVector::XMCOLOR( color.x, color.y, color.z, color.w );

    FW1_RECTF rect = { pos.x, pos.y, pos.x, pos.y };
    fontWrapper->AnalyzeString( nullptr, text, fontFamily, fontSize, &rect, color32,
        flags | FW1_NOFLUSH | FW1_NOWORDWRAP, textGeometry );
}