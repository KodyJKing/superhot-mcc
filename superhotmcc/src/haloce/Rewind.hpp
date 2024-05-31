#pragma once
#include "Halo1.hpp"

namespace Rewind {
    void snapshot( Halo1::EntityRecord* rec );
    void rewind( Halo1::EntityRecord* rec, float timescale, float globalTimescale );
}