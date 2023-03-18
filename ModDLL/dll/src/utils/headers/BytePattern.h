#include "../../../pch.h"

struct BytePattern {

    inline static const uint16_t wildcard = -1;

    std::vector<uint16_t> tokens;

    BytePattern( std::string pattern );

    bool compare( UINT_PTR address );

};

bool assertBytes( UINT_PTR address, std::string pattern );

bool assertBytes( const char* description, UINT_PTR address, std::string pattern );