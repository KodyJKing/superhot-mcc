#pragma once

template <typename Ty>
inline void safeRelease( Ty& comPtr ) {
    static_assert( std::is_pointer<Ty>::value,
        "safeRelease - comPtr not a pointer." );

    static_assert( std::is_base_of<IUnknown, std::remove_pointer<Ty>::type>::value,
        "safeRelease - remove_ptr<comPtr>::type is not a com object." );

    if ( comPtr ) {
        comPtr->Release();
        comPtr = nullptr;
    }
}

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