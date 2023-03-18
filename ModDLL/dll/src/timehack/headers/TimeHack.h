#include "../../../pch.h"

namespace TimeHack {

    extern float timeElapsed;

    bool init( uint64_t halo1Base );
    void cleanup();
    void onDllThreadUpdate();
    void onGameThreadUpdate();

}