#include "headers/Config.h"
#include "headers/common.h"

using std::string;

namespace Config {

    std::string getString( const char* section, const char* name, const char* defaultValue ) {
        std::string path = getModDirectory() + "\\SuperhotMCC.ini";
        char result[256];
        GetPrivateProfileStringA( section, name, defaultValue, result, ARRAYSIZE( result ), path.c_str() );
        return std::string( result );
    }

    float getFloat( const char* section, const char* name, float defaultValue ) {
        auto resultStr = getString( section, name, nullptr );
        try {
            return std::stof( resultStr, nullptr );
        } catch ( const std::exception& e ) {
            std::cout << "Error parsing option " << section << '.' << name << ": " << resultStr << "\n";
            std::cout << "    " << e.what() << '\n';
            return defaultValue;
        }
    }

    uint64_t getUint64( const char* section, const char* name, uint64_t defaultValue, int base ) {
        auto resultStr = getString( section, name, nullptr );
        try {
            return std::stoull( resultStr, nullptr, base );
        } catch ( const std::exception& e ) {
            std::cout << "Error parsing option " << section << '.' << name << ": " << resultStr << "\n";
            std::cout << "    " << e.what() << '\n';
            return defaultValue;
        }
    }

}