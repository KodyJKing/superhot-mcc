#pragma once

template <typename T>
inline void safeRelease( T& comPtr ) {
    static_assert( std::is_pointer<T>::value, "safeRelease - comPtr not a pointer." );
    static_assert( std::is_base_of<IUnknown, std::remove_pointer<T>::type>::value,
        "safeRelease - remove_ptr<comPtr>::type is not a com object." );
    if ( comPtr ) {
        comPtr->Release();
        comPtr = nullptr;
    }
}

template <typename T>
inline void safeFree( T& ptr ) {
    static_assert( std::is_pointer<T>::value, "safeRelease - ptr not a pointer." );
    if ( ptr ) {
        free( ptr );
        ptr = nullptr;
    }
}

void throwIfFail( const char* taskDescription, HRESULT hr );
void throwIfFail( HRESULT hr );