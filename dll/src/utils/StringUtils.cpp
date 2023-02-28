#include "../../pch.h"
#include "./headers/StringUtils.h"

using std::string;
using std::vector;

namespace StringUtils {

    bool checkCStr( const char* str, size_t maxLength ) {
        return str && strnlen( str, maxLength ) < maxLength;
    }

    vector<string> split( string str, string delim ) {
        vector<string> result;
        size_t pos = 0;
        bool lastSubstr = false;
        while ( !lastSubstr ) {
            size_t delimPos = str.find( delim, pos );
            lastSubstr = delimPos == string::npos;
            size_t substrLen = lastSubstr ? str.length() - pos : delimPos - pos;
            result.push_back( str.substr( pos, delimPos - pos ) );
            pos = delimPos + delim.length();
        }
        return result;
    }

}