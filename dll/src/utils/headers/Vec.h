#include "../../../pch.h"

namespace Vec {
    void print( Vec2 v );
    void print( Vec3 v );
    void print( Vec4 v );
    void print( XMMATRIX m );

    Vec2 add( Vec2 a, Vec2 b );
    Vec2 sub( Vec2 a, Vec2 b );
    Vec2 scale( Vec2 a, float s );
    Vec2 scale( Vec2 a, float x, float y );
    float dot( Vec2 a, Vec2 b );

    Vec3 add( Vec3 a, Vec3 b );
    Vec3 addScaled( Vec3 a, Vec3 b, float s );
    Vec3 sub( Vec3 a, Vec3 b );
    Vec3 subScaled( Vec3 a, Vec3 b, float s );
    Vec3 scale( Vec3 a, float s );
    float dot( Vec3 a, Vec3 b );
    float length( Vec3 a );
    Vec3 unit( Vec3 a );
    Vec3 lerp( Vec3 a, Vec3 b, float t );
    Vec3 rejection( Vec3 v, Vec3 axis );
    Vec3 cross( Vec3 a, Vec3 b );

    void scaleMut( Vec3& a, float scale );
    void clampMut( Vec3& a, float maxLength );
}