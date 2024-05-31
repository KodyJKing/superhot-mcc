#include "Vectors.hpp"

std::string Vec3::toString() {
    return "Vec( " + std::to_string( x ) + ", " + std::to_string( y ) + ", " + std::to_string( z ) + " )";
}

Vec3 Vec3::operator+( const Vec3& other ) { return Vec3{ x + other.x, y + other.y, z + other.z }; }
Vec3 Vec3::operator-( const Vec3& other ) { return Vec3{ x - other.x, y - other.y, z - other.z }; }
Vec3 Vec3::operator*( float scalar ) { return Vec3{ x * scalar, y * scalar, z * scalar }; }
Vec3 Vec3::operator/( float scalar ) { return Vec3{ x / scalar, y / scalar, z / scalar }; }

Vec3& Vec3::operator+=( const Vec3& other ) { x += other.x; y += other.y; z += other.z; return *this; }
Vec3& Vec3::operator-=( const Vec3& other ) { x -= other.x; y -= other.y; z -= other.z; return *this; }
Vec3& Vec3::operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; return *this; }
Vec3& Vec3::operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; return *this; }

Vec3 Vec3::cross( const Vec3& other ) {
    return Vec3{
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

float Vec3::dot( const Vec3& other ) {
    return x * other.x + y * other.y + z * other.z;
}

float Vec3::length() {
    return sqrt( x * x + y * y + z * z );
}

Vec3 Vec3::normalize() {
    float len = length();
    return Vec3{ x / len, y / len, z / len };
}

Vec3 Vec3::rejection(Vec3 axis) {
    return *this - axis * (this->dot(axis) / axis.dot(axis));
}

Vec3 Vec3::lerp( Vec3& a, Vec3& b, float t ) {
    return a + (b - a) * t;
}