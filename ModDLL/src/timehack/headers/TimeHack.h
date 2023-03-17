#include "../../../pch.h"

namespace TimeHack {

    extern float timeElapsed;

    void init( uint64_t halo1Base );
    void cleanup();
    void onDllThreadUpdate();
    void onGameThreadUpdate();

}