#pragma once

namespace StringUtils {

    bool checkCStr( const char* str, size_t maxLength );
    std::vector<std::string> split( std::string str, std::string delim );

}