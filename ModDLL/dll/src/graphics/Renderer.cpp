//
// Thanks to Yazzn for his renderer, which I used for reference here.
// https://www.unknowncheats.me/forum/d3d-tutorials-and-source/177926-direct3d-11-renderer.html
//

// TODO: Move FW1 code into TextRenderer class.

#include "./headers/DX11Utils.h"
#include "./headers/Renderer.h"
#include "./headers/shader.h"
#include "../utils/headers/common.h"
#include "../utils/headers/Vec.h"

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
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
        ZeroMemory( &desc, sizeof( desc ) );
        desc.RenderTarget->BlendEnable = true;
        desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
        // desc.RenderTarget->SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        throwIfFail( pDevice->CreateBlendState( &desc, &blendState ) );
    }

    { // Create no cull raster state
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = 0;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthBiasClamp = 0.0f;
        desc.DepthClipEnable = true;
        desc.ScissorEnable = false;
        desc.MultisampleEnable = false;
        // desc.AntialiasedLineEnable = false;
        desc.AntialiasedLineEnable = true;
        throwIfFail( pDevice->CreateRasterizerState( &desc, &noCullRasterState ) );
    }

    { // Create reverse depth-stencil state
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        throwIfFail( pDevice->CreateDepthStencilState( &desc, &depthStencilState ) );
    }

    { // Create reverse depth-stencil state
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        throwIfFail( pDevice->CreateDepthStencilState( &desc, &reverseDepthStencilState ) );
    }

    auto ident = XMMatrixIdentity();
    this->setTransform( &ident );

    // FW1
    this->defaultFontFamily = defaultFontFamily;
    throwIfFail( FW1CreateFactory( FW1_VERSION, &fontFactory ) );
    throwIfFail( fontFactory->CreateFontWrapper( pDevice, defaultFontFamily, &fontWrapper ) );
    throwIfFail( fontFactory->CreateTextGeometry( &textGeometry ) );
    hasTextToFlush = false;
}

Renderer::~Renderer() {
    safeRelease( pTransformBuffer );
    safeRelease( pVertexBuffer );
    safeRelease( vertLayout );
    safeRelease( VS );
    safeRelease( PS );
    safeRelease( blendState );
    safeRelease( reverseDepthStencilState );
    safeRelease( depthStencilState );
    safeRelease( noCullRasterState );
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
    if ( topology != this->topology ) {
        this->flush();
        this->topology = topology;
    }
}

void Renderer::setDepthReverse( bool reverse ) {
    pCtx->OMSetDepthStencilState( reverse ? reverseDepthStencilState : depthStencilState, 0 );
}

void Renderer::begin() {
    pCtx->VSSetShader( VS, 0, 0 );
    pCtx->PSSetShader( PS, 0, 0 );

    pCtx->OMSetBlendState( blendState, nullptr, 0xFFFFFFFF );
    pCtx->RSSetState( noCullRasterState );

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
    flush();
    vertexCount = 0;
    // FW1
    textGeometry->Clear();
}

void Renderer::flush() {
    if ( vertexCount > 0 ) {
        size_t vertexBytes = vertexCount * sizeof( Vertex );
        copyToBuffer( pCtx, pVertexBuffer, (void*) pCpuVertexBuffer, vertexBytes );
        pCtx->IASetPrimitiveTopology( topology );
        pCtx->Draw( vertexCount, 0 );
        vertexCount = 0;
    }

    // FW1
    if ( hasTextToFlush ) {
        fontWrapper->Flush( pCtx );
        fontWrapper->DrawGeometry( pCtx, textGeometry,
            nullptr, nullptr, FW1_RESTORESTATE );
        hasTextToFlush = false;
    }
}

void Renderer::pushVerticies( uint32_t pushCount, Vertex* pVertices ) {
    if ( vertexCount + pushCount > maxVertices )
        this->flush();
    auto pVertex = pCpuVertexBuffer + vertexCount;
    size_t pushBytes = pushCount * sizeof( Vertex );
    std::memcpy( (void*) pVertex, pVertices, pushBytes );
    vertexCount += pushCount;
}

// === Drawing Helpers ===

void Renderer::drawLine( Vertex a, Vertex b ) {
    setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
    pushVerticies( 1, &a );
    pushVerticies( 1, &b );
    flush();
}

void Renderer::drawThickLine( Vertex a, Vertex b, float thickness, Vec3 viewPosition ) {
    setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    Vec3 midPoint = Vec::scale( Vec::add( a.pos, b.pos ), 0.5f );
    Vec3 viewDirection = Vec::unit( Vec::sub( midPoint, viewPosition ) );

    Vec3 u = Vec::unit( Vec::sub( b.pos, a.pos ) );
    Vec3 v = Vec::unit( Vec::cross( u, viewDirection ) );

    Vertex vertices[] = {
        { Vec::add( a.pos, Vec::scale( v, thickness ) ), a.color },
        { Vec::add( a.pos, Vec::scale( v, -thickness ) ), a.color },
        { Vec::add( b.pos, Vec::scale( v, thickness ) ), a.color },
        { Vec::add( b.pos, Vec::scale( v, -thickness ) ), a.color }
    };

    pushVerticies( ARRAYSIZE( vertices ), vertices );
    flush();
}

// === FW1 ===

void Renderer::drawText( Vec2 pos, LPCWSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily ) {

    if ( !fontFamily ) fontFamily = defaultFontFamily;

    uint32_t color32 = PackedVector::XMCOLOR( color.z, color.y, color.x, color.w );

    FW1_RECTF rect = { pos.x, pos.y, pos.x, pos.y };
    fontWrapper->AnalyzeString( nullptr, text, fontFamily, fontSize, &rect, color32,
        flags | FW1_NOFLUSH | FW1_NOWORDWRAP, textGeometry );

    hasTextToFlush = true;

}

void Renderer::drawText( Vec2 pos, LPCSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily ) {
    std::wstringstream strm;
    strm << text;
    drawText( pos, strm.str().c_str(), color, flags, fontSize, fontFamily );
}

Vec2 Renderer::measureText( LPCWSTR text, float fontSize, LPCWSTR fontFamily ) {

    if ( !fontFamily ) fontFamily = defaultFontFamily;

    FW1_RECTF nullRect = { 0.f, 0.f, 0.f, 0.f };
    FW1_RECTF rect = fontWrapper->MeasureString( text, fontFamily,
        fontSize, &nullRect, FW1_NOWORDWRAP );

    return { rect.Right, rect.Bottom };

}