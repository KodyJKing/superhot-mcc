#include "../../../pch.h"
#include <DirectXMath.h>

using namespace DirectX;

using Vector3 = XMFLOAT3;
using Vector4 = XMFLOAT4;

struct Vertex {
    Vector3 pos;
    Vector4 color;
};

class Renderer {

    public:
    Renderer( ID3D11Device* pDevice, uint32_t maxVertices );
    ~Renderer();

    void setTransform( XMMATRIX* pTransform );
    void setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topology );

    void begin();
    void end();

    void pushVerticies( uint32_t pushCount, Vertex* pVertices );
    void flush();

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
};
