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

void throwIfFail( const char* taskDescription, HRESULT hr );
void throwIfFail( HRESULT hr );