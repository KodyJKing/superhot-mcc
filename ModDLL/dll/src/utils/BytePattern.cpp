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
                std::cout << "Bad argument to byte pattern: \"" << part;
                std::cout << "\" (" << e.what() << ")\n";
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

#define FMT_BYTE std::uppercase << std::hex << std::setw(2) << std::setfill('0')

bool assertBytes( const char* description, UINT_PTR address, std::string pattern ) {
    BytePattern bp( pattern );
    auto result = bp.compare( address );

    if ( !result ) {
        if ( description )
            std::cout << "\nByte check failed: " << description << '\n';

        std::cout << "    Expected: ";
        for ( auto token : bp.tokens ) {
            if ( token == BytePattern::wildcard )
                std::cout << "?? ";
            else
                std::cout << FMT_BYTE << token << " ";
        }

        std::cout << "\n    Found:    ";
        for ( int i = 0; i < bp.tokens.size(); i++ ) {
            uint16_t byte = (uint16_t) ( *(uint8_t*) ( address + i ) );
            std::cout << FMT_BYTE << byte << " ";
        }
        std::cout << "\n\n";
    }


    return result;
}
