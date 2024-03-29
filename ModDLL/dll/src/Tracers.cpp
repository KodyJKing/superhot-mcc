#include "headers/Tracers.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"
#include "utils/headers/MathUtils.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"

using namespace Halo1;
using MathUtils::smoothstep;
using std::vector;
using std::string;
using std::unordered_map;

namespace Tracers {

    // struct PositionShift {
    //     float absolute = 0.0f;
    //     float velocity = 0.0f;
    // };

    // PositionShift getPositionShift( string projectileResourceName ) {

    //     static unordered_map<string, PositionShift> shifts {
    //         { "weapons\\plasma pistol\\bolt",
    //             { .absolute = 1.0f } },
    //         { "weapons\\needler\\needle",
    //             { .absolute = 0.133f } },
    //         { "weapons\\plasma rifle\\bolt",
    //             { .velocity = 1.0f } },
    //         { "weapons\\sniper rifle\\sniper bullet",
    //             { .velocity = 1.0f } },
    //         { "weapons\\assault rifle\\bullet",
    //             { .velocity = 1.0f } },
    //         { "weapons\\pistol\\bullet",
    //             { .velocity = 1.0f } },
    //     };

    //     if ( shifts.count( projectileResourceName ) == 0 )
    //         return { 0.0f, 0.0f };

    //     return shifts[projectileResourceName];
    // }

    struct TracerPoint {
        Vec3 pos;
        float creationTime;
    };

    struct Tracer {
        uint32_t projectileHandle;
        vector<TracerPoint> points;
        float lastUpdated;

        Tracer() {}
        Tracer( uint32_t projectileHandle ) {
            this->projectileHandle = projectileHandle;
            points.reserve( 128 );
        }

        void reset( uint32_t newHandle ) {
            projectileHandle = newHandle;
            points.clear();
        }

        float length() {
            float result = 0.0f;
            for ( size_t i = 1; i < points.size(); i++ ) {
                auto p = points[i].pos;
                auto q = points[i - 1].pos;
                result += Vec::length( Vec::sub( p, q ) );
            }
            return result;
        }

        void draw( Renderer* renderer, float time, Vec3 viewPos ) {

            static const float fadeTime = 500.0f;

            if ( time - lastUpdated > fadeTime )
                return;

            renderer->setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            Vertex verts[2];

            float netLength = length();
            float lengthSoFar = 0.0f;

            for ( size_t i = 1; i < points.size(); i++ ) {

                auto p = points[i];
                auto q = points[i - 1];

                lengthSoFar += Vec::length( Vec::sub( p.pos, q.pos ) );

                Vec3 viewDiff = Vec::sub( p.pos, viewPos );
                Vec3 viewDir = Vec::unit( viewDiff );
                float viewDist = Vec::length( viewDiff );
                Vec3 u = Vec::unit( Vec::sub( p.pos, q.pos ) );
                Vec3 v = Vec::unit( Vec::cross( u, viewDir ) );

                float age = ( time - p.creationTime ) / fadeTime;
                float normalizedLength = lengthSoFar / netLength;
                float alphaLen = smoothstep( 0.0f, 1.0f, normalizedLength ) * sqrtf( 1.0f - normalizedLength * normalizedLength );
                float alphaAge = smoothstep( 1.0f, 0.0f, age );
                Vec4 color = Colors::withAlpha( Colors::red, sqrtf( alphaLen * alphaAge ) );
                float width = alphaLen * 0.025f;

                verts[0] = { Vec::add( p.pos, Vec::scale( v, width ) ), color };
                verts[1] = { Vec::add( p.pos, Vec::scale( v, -width ) ), color };

                renderer->pushVerticies( 2, verts );

            }

            renderer->flush();

        }

        void update( Entity* entity, uint32_t handle, float time ) {
            if ( handle != projectileHandle )
                reset( handle );

            Vec3 p1;
            auto N = points.size();
            if ( N == 0 ) {
                p1 = entity->pos;
            } else {
                // auto shift = getPositionShift( entity->getTagResourcePath() );
                // p1 = Vec::addScaled(
                //     Vec::addScaled( entity->pos, entity->vel, shift.velocity ),
                //     Vec::unit( entity->vel ), shift.absolute
                // );
                p1 = entity->pos;
                auto p0 = points[N - 1].pos;
                if ( Vec::distance( p0, p1 ) < 0.0001 )
                    return;
            }

            points.emplace_back( p1, time );
            lastUpdated = time;
        }
    };


    void render( Renderer* renderer, float time, ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        static std::map<uint16_t, Tracer> tracers;

        XMMATRIX transform;
        auto screenDimensions = HaloMCC::getWindowSize();
        if ( FAILED( Halo1::getCameraMatrix( screenDimensions.x, screenDimensions.y, transform ) ) ) return;
        renderer->setTransform( &transform );

        fitViewportToWindow( pCtx, HaloMCC::getWindow() );
        renderer->setDepthReverse( true );

        foreachEntityRecordIndexed(
            [time]( EntityRecord* rec, uint16_t i ) {
                auto entity = rec->entity();
                if ( !entity || ( entity->entityCategory != EntityCategory_Projectile ) )
                    return;
                auto handle = indexToEntityHandle( i );
                if ( tracers.count( i ) == 0 )
                    tracers[i] = Tracer( handle );
                tracers[i].update( entity, handle, time );
            }
        );

        renderer->begin();
        auto pCam = getPlayerCameraPointer();
        for ( auto& [key, tracer] : tracers )
            tracer.draw( renderer, time, pCam->pos );
        renderer->end();

    }

}
