#include "headers/BytePattern.h"
#include "headers/StringUtils.h"

BytePattern::BytePattern( std::string pattern ) {
    auto parts = StringUtils::split( pattern, " " );
    for ( auto part : parts ) {
        if ( part == "??" ) {
            tokens.push_back( BytePattern::wildcard );
        } else {
            try {
                uint64_t byte = std::stoul( part, nullptr, 16 );
                tokens.push_back( (uint16_t) byte );
            } catch ( const std::exception& e ) {
                tokens.push_back( BytePattern::wildcard );
                std::cerr << "Bad argument to byte pattern: \"" << part;
                std::cerr << "\" (" << e.what() << ")\n";
            }
        }
    }
}

bool BytePattern::compare( UINT_PTR address ) {
    uint8_t* head = (uint8_t*) address;
    for ( auto token : tokens ) {
        if ( token != BytePattern::wildcard && *head != (uint8_t) token )
            return false;
        head++;
    }
    return true;
}

bool assertBytes( UINT_PTR address, std::string pattern ) {
    return assertBytes( nullptr, address, pattern );
}

bool assertBytes( const char* description, UINT_PTR address, std::string pattern ) {
    BytePattern bp( pattern );
    auto result = bp.compare( address );
    if ( !result && description )
        std::cerr << "Byte check failed: " << description << '\n';
    return result;
}
