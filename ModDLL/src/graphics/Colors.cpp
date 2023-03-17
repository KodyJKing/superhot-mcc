#include "headers/Colors.h"

namespace Colors {
    const Vec4
        red{ 1.0f, 0.0f, 0.0f, 1.0f },
        green{ 0.0f, 1.0f, 0.0f, 1.0f },
        blue{ 0.0f, 0.0f, 1.0f, 1.0f },
        black{ 0.0f, 0.0f, 0.0f, 1.0f },
        white{ 1.0f, 1.0f, 1.0f, 1.0f },
        clear{ 0.0f, 0.0f, 0.0f, 0.0f };

    Vec4 withAlpha( Vec4 color, float alpha ) { return { color.x, color.y, color.z, alpha }; }
}