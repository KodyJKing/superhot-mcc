#pragma once

#include "../../../pch.h"

namespace Config {

    std::string getString( const char* section, const char* name, const char* defaultValue );

    float getFloat( const char* section, const char* name, float defaultValue );

    uint64_t getUint64( const char* section, const char* name, uint64_t defaultValue, int base = 10 );

}