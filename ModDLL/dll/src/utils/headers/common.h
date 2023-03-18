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

template <typename T>
bool isZero( T* pData ) {
    const size_t size = sizeof( T );
    uint8_t* pBytes = (uint8_t*) pData;
    for ( int i = 0; i < size; i++ )
        if ( pBytes[i] != 0 )
            return false;
    return true;
}

void throwIfFail( const char* taskDescription, HRESULT hr );
void throwIfFail( HRESULT hr );

void showAndPrintError( const char* message );

void updateFloat( const char* floatName, float& x, float rate, int increaseKey, int decreaseKey );

bool keypressed( char vk );

void toggleOption( const char* description, bool& option, int key );

void memcpyExecutable( char* dest, char* source, size_t size );

void copyANSITextToClipboard( const char* text, size_t maxLength = 500 );

bool debounceCheck( uint64_t& unblockTime, uint64_t blockMilis );
void printDebounced( uint64_t& unblockTime, uint64_t blockMilis, const char* text );