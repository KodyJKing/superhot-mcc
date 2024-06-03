#include "Strings.hpp"
#include <stdint.h>

namespace Strings {

    std::string fourccToString(uint32_t fourcc) {
        char str[5];
        str[0] = (fourcc >> 24) & 0xFF;
        str[1] = (fourcc >> 16) & 0xFF;
        str[2] = (fourcc >> 8) & 0xFF;
        str[3] = fourcc & 0xFF;
        str[4] = 0;
        return std::string(str);
    }

}