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