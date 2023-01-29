#include "../../../pch.h"
#include <DirectXMath.h>

using namespace DirectX;

class Renderer {

    public:
    Renderer( ID3D11Device* pDevice, uint32_t vboSize );
    ~Renderer();

    void setTransform( XMMATRIX transform );

    void begin();
    void end();

    void pushVerticies( uint32_t count, void* pVertices );
    void flush();

    private:
    XMMATRIX transform;
    ID3D11Device* pDevice;
    ID3D11Buffer* pVertexBuffer;
    ID3D11Buffer* pTransformBuffer;
};

const char* shader;