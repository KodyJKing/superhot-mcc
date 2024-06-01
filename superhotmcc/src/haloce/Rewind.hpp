#pragma once
#include "Halo1.hpp"

namespace Rewind {
    void snapshot( Halo1::EntityRecord* rec, Halo1::Entity& snap );
    void rewind( Halo1::EntityRecord* rec, float timescale, float globalTimescale, Halo1::Entity& snap );
}