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
        float creationTime;
    };

    static size_t bufferWriteHead;
    static TracerLine lineBuffer[4096];

    void addLine( Vec3 a, Vec3 b, float time ) {
        lineBuffer[bufferWriteHead++] = { a, b, time };
        bufferWriteHead %= ARRAYSIZE( lineBuffer );
    }

    // =========================================

    void render( Renderer* renderer, float time, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        XMMATRIX transform;
        auto screenDimensions = HaloMCC::getWindowSize();
        if ( FAILED( Halo1::getCameraMatrix( screenDimensions.x, screenDimensions.y, transform ) ) ) return;
        renderer->setTransform( &transform );

        fitViewportToWindow( pCtx, HaloMCC::getWindow() );
        renderer->setDepthReverse( true );

        foreachEntityRecordIndexed(
            [renderer, time]( EntityRecord* rec, uint16_t i ) {
                auto entity = rec->entity();
                if ( !entity || ( entity->entityCategory != EntityCategory_Projectile ) )
                    return;

                ProjectileData currentData = { entity->projectileAge, entity->pos, true };
                ProjectileData oldData = projectileData[i];

                // If the age decreased, then the entity was deleted and a new one took its place.
                if ( oldData.initialized && oldData.age < currentData.age )
                    addLine( oldData.pos, currentData.pos, time );

                projectileData[i] = currentData;
            }
        );

        renderer->begin();
        for ( size_t i = 0; i < ARRAYSIZE( lineBuffer ); i++ ) {
            static const float fadeTime = 250.0f;
            TracerLine line = lineBuffer[i];
            float age = time - line.creationTime;
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
