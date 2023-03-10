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


bool keypressed( char vk ) {
    static bool wasPressed[0xFF] = {};
    int isPressed = GetAsyncKeyState( vk ) != 0;
    int result = !wasPressed[vk] && isPressed;
    wasPressed[vk] = isPressed;
    return result;
}

void toggleOption( const char* description, bool& option, int key ) {
    if ( keypressed( key ) ) {
        std::cout << description << ( option ? " disabled.\n" : " enabled.\n" );
        option = !option;
    }
}

void memcpyExecutable( char* dest, char* source, size_t size ) {
    DWORD oldProtect;
    VirtualProtect( (void*) dest, size, PAGE_EXECUTE_READWRITE, &oldProtect );
    memcpy( dest, source, size );
    VirtualProtect( (void*) dest, size, oldProtect, &oldProtect );
}

void copyANSITextToClipboard( const char* text, size_t maxLength ) {
    auto length = strnlen( text, maxLength );
    if ( length >= maxLength ) {
        std::cout << "Tried to copy too many chars to clipboard. Null terminator was not found.\n";
        return;
    }

    auto byteCount = length + 1;
    HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE, byteCount );
    memcpy( GlobalLock( hMem ), text, byteCount );
    GlobalUnlock( hMem );

    OpenClipboard( NULL );
    EmptyClipboard();
    SetClipboardData( CF_TEXT, hMem );
    CloseClipboard();
}

bool debounceCheck( uint64_t& unblockTime, uint64_t blockMilis ) {
    uint64_t t = GetTickCount64();
    if ( t < unblockTime )
        return false;
    unblockTime = t + blockMilis;
    return true;
}

void printDebounced( uint64_t& unblockTime, uint64_t blockMilis, const char* text ) {
    if ( debounceCheck( unblockTime, blockMilis ) )
        std::cout << text;
}
