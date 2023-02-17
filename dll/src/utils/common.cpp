#include "../../pch.h"
#include "./headers/common.h"

void throwIfFail( const char* taskDescription, HRESULT hr ) {
    if ( FAILED( hr ) ) {
        std::stringstream ss;
        ss << "Error: " << hr;
        if ( taskDescription )
            ss << " while: " << taskDescription;
        throw std::runtime_error( ss.str() );
    }
}

void throwIfFail( HRESULT hr ) {
    throwIfFail( nullptr, hr );
}

void updateFloat( const char* floatName, float& x, float rate, int increaseKey, int decreaseKey ) {
    int increase = GetAsyncKeyState( increaseKey );
    int decrease = GetAsyncKeyState( decreaseKey );
    if ( increase ) x *= rate;
    if ( decrease ) x /= rate;
    if ( increase || decrease )
        std::cout << floatName << ": " << x << "\n";
}