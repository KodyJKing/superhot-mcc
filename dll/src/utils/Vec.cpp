#include "./headers/Vec.h"

namespace Vec {

    void print( Vec2 v ) {
        printf( "(%.2f, %.2f)", v.x, v.y );
    }

    void print( Vec3 v ) {
        printf( "(%.2f, %.2f, %.2f)", v.x, v.y, v.z );
    }

    void print( Vec4 v ) {
        printf( "(%.2f, %.2f, %.2f, %.2f)", v.x, v.y, v.z, v.w );
    }

    void print( XMMATRIX m ) {
        float* floats = reinterpret_cast<float*>( &m );
        for ( int i = 0; i < 4; i++ ) {
            for ( int j = 0; j < 4; j++ ) {
                int index = j * 4 + i;
                std::cout << std::setfill( ' ' ) << std::setw( 6 ) << std::fixed
                    << std::setprecision( 2 ) << floats[index] << " ";
                // printf( "%.2f ", floats[index] );
            }
            printf( "\n" );
        }
    }

}