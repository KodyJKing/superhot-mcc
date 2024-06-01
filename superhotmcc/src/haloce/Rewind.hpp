#pragma once
#include "Halo1.hpp"

namespace Rewind {

    struct Snapshot {
        Vec3 pos, vel, fwd, up, angularVelocity;
        float fuse, heat, projectileAge, shield;
        uint32_t parentHandle, ageMilis;
        uint16_t animFrame;
        uint8_t ticksSinceLastFired;
    };

    void snapshot( Halo1::EntityRecord* rec, Snapshot& snap );
    void rewind( Halo1::EntityRecord* rec, float timescale, float globalTimescale, Snapshot& snap );
}