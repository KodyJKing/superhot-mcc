#include "./headers/Vec3.h"

inline Vec3 Vec3::operator+( Vec3 v ) { return { x + v.x, y + v.y, z + v.z }; }
inline Vec3 Vec3::operator-( Vec3 v ) { return { x - v.x, y - v.y, z - v.z }; }
inline Vec3 Vec3::operator*( float s ) { return { x * s, y * s, z * s }; }
inline Vec3 Vec3::operator/( float s ) { return { x / s, y / s, z / s }; }

inline float Vec3::length() { return sqrtf( x * x + y * y + z * z ); }
inline float Vec3::lengthSq() { return x * x + y * y + z * z; }
inline float Vec3::dot( Vec3 v ) { return x * v.x + y * v.y + z * v.z; }
inline Vec3 Vec3::cross( Vec3 v ) {
    return {
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x,
    };
}
inline Vec3 Vec3::unit() {
    float invLen = 1.0f / ( this->length() );
    return { x * invLen, y * invLen, z * invLen };
}

inline Vec3 Vec3::lerp( Vec3 v, float t ) {
    return *this * ( 1 - t ) + v * t;
}

inline Vec3 Vec3::rejection( Vec3 axis ) {
    return *this - axis * this->dot( axis ) / axis.lengthSq();
}

inline Angles Vec3::toAngles() {
    float r = sqrtf( x * x + y * y );
    return {
        atan2f( y, x ),
        atan2f( z, r )
    };
}

void Vec3::print() {
    printf( "< %f.4, %f.4, %f.4 >", x, y, z );
}
