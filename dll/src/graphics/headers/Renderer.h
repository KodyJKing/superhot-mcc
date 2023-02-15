//
// Thanks to Yazzn for his renderer, which I used for extensively reference here.
// https://www.unknowncheats.me/forum/d3d-tutorials-and-source/177926-direct3d-11-renderer.html
//

#pragma once

#include "../../../pch.h"
#include <DirectXMath.h>

#include "FW1FontWrapper.h"
#pragma comment(lib, "FW1FontWrapper.lib")

using namespace DirectX;

using Vector2 = XMFLOAT2;
using Vector3 = XMFLOAT3;
using Vector4 = XMFLOAT4;

struct Vertex {
    Vector3 pos;
    Vector4 color;
};

class Renderer {

    public:
    Renderer( ID3D11Device* pDevice, uint32_t maxVertices, LPCWSTR defaultFontFamily = L"Verdana" );
    ~Renderer();

    void setTransform( XMMATRIX* pTransform );
    void setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topology );

    void begin();
    void end();

    void pushVerticies( uint32_t pushCount, Vertex* pVertices );
    void flush();

    // FW1
    void drawText( Vector2 pos, LPCWSTR text, Vector4 color, uint32_t flags,
        float fontSize, LPCWSTR fontFamily );

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

    // FW1
    LPCWSTR defaultFontFamily;
    IFW1Factory* fontFactory;
    IFW1FontWrapper* fontWrapper;
    IFW1TextGeometry* textGeometry;
};
