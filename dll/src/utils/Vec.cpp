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
            }
            printf( "\n" );
        }
    }

    Vec2 add( Vec2 a, Vec2 b ) { return { a.x + b.x, a.y + b.y }; }
    Vec2 sub( Vec2 a, Vec2 b ) { return { a.x - b.x, a.y - b.y }; }
    Vec2 scale( Vec2 a, float s ) { return { a.x * s, a.y * s }; }
    Vec2 scale( Vec2 a, float x, float y ) { return { a.x * x, a.y * y }; }
    float dot( Vec2 a, Vec2 b ) { return a.x * b.x + a.y * b.y; }

    Vec3 add( Vec3 a, Vec3 b ) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
    Vec3 sub( Vec3 a, Vec3 b ) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
    Vec3 scale( Vec3 a, float s ) { return { a.x * s, a.y * s, a.z * s }; }
    float dot( Vec3 a, Vec3 b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    float length( Vec3 a ) { return sqrtf( a.x * a.x + a.y * a.y + a.z * a.z ); }
    Vec3 unit( Vec3 a ) { return scale( a, 1 / length( a ) ); }

    void scaleMut( Vec3& a, float scale ) { a.x *= scale; a.y *= scale; a.z *= scale; }
    void clampMut( Vec3& a, float maxLength ) {
        float aLength = length( a );
        if ( aLength > maxLength )
            scaleMut( a, maxLength / aLength );
    }

}