#include "./headers/Overlay.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"
#include "utils/headers/MathUtils.h"
#include "graphics/headers/Renderer.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"

namespace Overlay {

    bool drawRandomTriangleOnEntity( Halo1::EntityRecord* rec );

    Renderer* renderer;

    void cleanup() {
        if ( renderer )
            renderer->~Renderer();
    }

    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        if ( !renderer ) {
            std::cout << "Constructing new Renderer.\n";
            renderer = new Renderer( pDevice, 4096 );
        }

        XMMATRIX transform;
        Vec2 dims = HaloMCC::getWindowSize();
        if ( FAILED( Halo1::getCameraMatrix( dims.x, dims.y, transform ) ) )
            return;

        renderer->setTransform( &transform );

        fitViewportToWindow( pCtx, HaloMCC::getWindow() );
        renderer->setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        renderer->begin();

        Halo1::foreachEntityRecord( drawRandomTriangleOnEntity );

        Vec4 color = { 1.0f, 1.0f, 1.0f, 0.25f };
        renderer->drawText( { 500, 100 }, L"Hello Text!", color, NULL, 100.0f, nullptr );

        renderer->flush();
        renderer->end();

    }

    bool drawRandomTriangleOnEntity( Halo1::EntityRecord* rec ) {
        auto entity = Halo1::getEntityPointer( rec );

        if ( entity->health <= 0 )
            return true;

        Vertex verticies[3] = { {{}, Colors::red }, {{}, Colors::green}, {{}, Colors::blue} };

        // #define RAND ( randf() * 2.0f - 1.0f ) * 0.1f
#define RAND MathUtils::guassian() * 0.02f
        auto pos = entity->pos;
        for ( int j = 0; j < 100; j++ ) {
            for ( int i = 0; i < 3; i++ ) {
                verticies[i].color.w = 0.25f;
                verticies[i].pos.x = pos.x + RAND;
                verticies[i].pos.y = pos.y + RAND;
                verticies[i].pos.z = pos.z + RAND;
            }
            renderer->pushVerticies( ARRAYSIZE( verticies ), verticies );
        }
#undef RAND

        return true;
    }

}