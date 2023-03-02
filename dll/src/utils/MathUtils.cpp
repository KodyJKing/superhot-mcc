#include "../../pch.h"
#include "./headers/MathUtils.h"

namespace MathUtils {

    float lerp( float start, float end, float t ) {
        return start * ( 1.0f - t ) + end * t;
    }

    float unlerp( float start, float end, float x ) {
        return ( x - start ) / ( end - start );
    }

    float clamp( float x, float min, float max ) { return x < min ? min : ( x > max ? max : x ); }

    float smoothstep( float edge0, float edge1, float x ) {
        x = clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.0f, 1.0f );
        return x * x * ( 3 - 2 * x );
    }

    INT_PTR signedDifference( UINT_PTR a, UINT_PTR b ) {
        if ( a > b ) return (INT_PTR) ( a - b );
        return -(INT_PTR) ( b - a );
    }

    float randf() {
        return (float) rand() / (float) RAND_MAX;
    }

    float guassian() {

        static float surplus = 0;
        static bool hasSurplus = false;

        if ( hasSurplus ) {
            hasSurplus = false;
            return surplus;
        }

        float u1 = randf();
        float u2 = randf();

        float k1 = sqrtf( -2.0f * logf( u1 ) );
        float k2 = (float) M_PI * 2.0f * u2;

        float z1 = k1 * cosf( k2 );
        float z2 = k1 * sinf( k2 );

        hasSurplus = true;
        surplus = z2;

        return z1;
    }

}