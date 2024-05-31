#include <string>

struct Vec3 {
    float x, y, z;

    std::string toString();

    Vec3 operator+( const Vec3& other );
    Vec3 operator-( const Vec3& other );
    Vec3 operator*( float scalar );
    Vec3 operator/( float scalar );

    Vec3& operator+=( const Vec3& other );
    Vec3& operator-=( const Vec3& other );
    Vec3& operator*=( float scalar );
    Vec3& operator/=( float scalar );

    Vec3 cross( const Vec3& other );
    float dot( const Vec3& other );

    float length();

    // Returns a normalized *copy* of this vector.
    Vec3 normalize();

    // Keeps the component of this vector that is perpendicular to the given axis.
    Vec3 rejection(Vec3 axis);

    static Vec3 lerp( Vec3& a, Vec3& b, float t );
};
