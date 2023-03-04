//
// Thanks to Yazzn for his renderer, which I used for reference here.
// https://www.unknowncheats.me/forum/d3d-tutorials-and-source/177926-direct3d-11-renderer.html
//

#pragma once

#include "../../../pch.h"

#include "FW1FontWrapper.h"
#pragma comment(lib, "FW1FontWrapper.lib")

struct Vertex {
    Vec3 pos;
    Vec4 color;
};

class Renderer {

    public:
    Renderer( ID3D11Device* pDevice, uint32_t maxVertices, LPCWSTR defaultFontFamily = L"Consolas" );
    ~Renderer();

    void setTransform( XMMATRIX* pTransform );
    void setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topology );
    void setDepthReverse( bool reverse );

    void begin();
    void end();

    void pushVerticies( uint32_t pushCount, Vertex* pVertices );
    void flush();

    // === Drawing Helpers ===
    void drawLine( Vertex a, Vertex b );
    void drawThickLine( Vertex a, Vertex b, float thickness, Vec3 viewDirection );

    // === FW1 ===
    void drawText( Vec2 pos, LPCWSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily );
    void drawText( Vec2 pos, LPCSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily );
    Vec2 measureText( LPCWSTR text, float fontSize, LPCWSTR fontFamily );

    private:
    ID3D11Device* pDevice;
    ID3D11DeviceContext* pCtx;

    ID3D11Buffer* pTransformBuffer;

    uint32_t vboSize;
    uint32_t maxVertices;
    uint32_t vertexCount;
    Vertex* pCpuVertexBuffer;
    ID3D11Buffer* pVertexBuffer;

    D3D11_PRIMITIVE_TOPOLOGY topology;

    ID3D11InputLayout* vertLayout;
    ID3D11VertexShader* VS;
    ID3D11PixelShader* PS;

    ID3D11BlendState* blendState;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11DepthStencilState* reverseDepthStencilState;
    ID3D11RasterizerState* noCullRasterState;

    // FW1
    LPCWSTR defaultFontFamily;
    IFW1Factory* fontFactory;
    IFW1FontWrapper* fontWrapper;
    IFW1TextGeometry* textGeometry;
    bool hasTextToFlush;
};
