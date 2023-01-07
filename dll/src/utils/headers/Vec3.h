#pragma once
#include "../../../pch.h"

typedef struct { float yaw, pitch; } Angles;

struct Vec3 {
    float x, y, z;
    inline Vec3 operator+( Vec3 v );
    inline Vec3 operator-( Vec3 v );
    inline Vec3 operator*( float s );
    inline Vec3 operator/( float s );

    inline float length();
    inline float lengthSq();
    inline float dot( Vec3 v );
    inline Vec3 cross( Vec3 v );
    inline Vec3 unit();

    inline Vec3 lerp( Vec3 v, float t );

    inline Vec3 rejection( Vec3 axis );

    inline Angles toAngles();

    void print();
};
