#include "headers/Tracers.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"
#include "utils/headers/Vec.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"

using namespace Halo1;

namespace Tracers {

    // =========================================

    struct ProjectileData {
        float age;
        Vec3 pos;
        bool initialized;
    };

    static std::unordered_map<uint32_t, ProjectileData> projectileData;

    // =========================================

    struct TracerLine {
        Vec3 a, b;
        uint64_t creationTick;
    };

    static size_t bufferWriteHead;
    static TracerLine lineBuffer[4096];

    void addLine( Vec3 a, Vec3 b ) {
        lineBuffer[bufferWriteHead++] = { a, b, GetTickCount64() };
        bufferWriteHead %= ARRAYSIZE( lineBuffer );
    }

    // =========================================

    void render( Renderer* renderer, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        XMMATRIX transform;
        auto screenDimensions = HaloMCC::getWindowSize();
        if ( FAILED( Halo1::getCameraMatrix( screenDimensions.x, screenDimensions.y, transform ) ) ) return;
        renderer->setTransform( &transform );

        fitViewportToWindow( pCtx, HaloMCC::getWindow() );
        renderer->setDepthReverse( true );

        // renderer->begin();
        foreachEntityRecordIndexed(
            [&renderer]( EntityRecord* rec, uint32_t i ) {
                auto entity = rec->entity();
                if ( !entity || ( entity->entityCategory != EntityCategory_Projectile ) )
                    return;

                ProjectileData currentData = { entity->projectileAge, entity->pos, true };
                ProjectileData oldData = projectileData[i];

                // If the age decreased, then the entity was deleted and a new one took its place.
                if ( oldData.initialized && oldData.age < currentData.age ) {
                    // auto pos1 = currentData.pos;
                    // auto pos0 = oldData.pos;
                    // Vertex vert1 = { pos1, Colors::red };
                    // Vertex vert0 = { pos0, Colors::withAlpha( Colors::red, 0.0f ) };
                    // renderer->drawLine( vert0, vert1 );

                    addLine( oldData.pos, currentData.pos );
                }

                projectileData[i] = currentData;
            }
        );

        renderer->begin();

        uint64_t time = GetTickCount64();
        for ( size_t i = 0; i < ARRAYSIZE( lineBuffer ); i++ ) {
            static const uint64_t fadeTime = 1000;
            TracerLine line = lineBuffer[i];
            uint64_t age = time - line.creationTick;
            if ( age > fadeTime )
                continue;
            float fadeProgress = (float) age / (float) fadeTime;
            Vec4 color = Colors::withAlpha( Colors::red, 1.0f - fadeProgress );
            renderer->drawLine(
                { line.a, color },
                { line.b, color }
            );
        }

        renderer->flush();
        renderer->end();

    }

}
