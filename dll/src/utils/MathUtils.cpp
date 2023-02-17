#include "../../pch.h"
#include "./headers/MathUtils.h"

namespace MathUtils {

    float lerp( float start, float end, float t ) {
        return start * ( 1.0f - t ) + end * t;
    }

    float unlerp( float start, float end, float x ) {
        return ( x - start ) / ( end - start );
    }

    INT_PTR signedDifference( UINT_PTR a, UINT_PTR b ) {
        if ( a > b ) return (INT_PTR) ( a - b );
        return -(INT_PTR) ( b - a );
    }

    float randf() {
        return (float) rand() / (float) RAND_MAX;
    }

    float _guassianSurplus = 0;
    bool _guassianHasSurplus = false;
    float guassian() {
        if ( _guassianHasSurplus ) {
            _guassianHasSurplus = false;
            return _guassianSurplus;
        }

        float u1 = randf();
        float u2 = randf();

        float k1 = sqrtf( -2.0f * logf( u1 ) );
        float k2 = (float) M_PI * 2.0f * u2;

        float z1 = k1 * cosf( k2 );
        float z2 = k1 * sinf( k2 );

        _guassianHasSurplus = true;
        _guassianSurplus = z2;

        return z1;
    }

}