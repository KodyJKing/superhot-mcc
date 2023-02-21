#include "./headers/Rewind.h"
#include "../headers/Halo1.h"
#include "../utils/headers/common.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;

namespace Rewind {

    #define REWIND(field, type) \
        { auto delta_##field = entity->field - old_##field; \
        entity->field = old_##field + (type)(delta_##field * timescale) }

    #define REWIND_VEC(field, type) \
        { type delta_##field = Vec::sub(entity->field, old_##field); \
        entity->field = Vec::add(old_##field, Vec::scale(delta_##field, timescale)); }

    #define REWIND_INCREASES(field, type) \
        { auto delta_##field = entity->field - old_##field; \
        if (delta_##field > (type) 0) \
            entity->field = old_##field + (type)(delta_##field * timescale) }

    #define REWIND_DECREASES(field, type) \
        { auto delta_##field = entity->field - old_##field; \
        if (delta_##field < (type) 0) \
            entity->field = old_##field + (type)(delta_##field * timescale) }

    #define SAVE(field) old_##field = entity->field

    // Fields
    Vec3 old_pos, old_vel;
    uint32_t old_parentHandle;
    uint16_t old_animFrame;

    void snapshot( EntityRecord* rec ) {

        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;

        SAVE( pos );
        SAVE( vel );
        SAVE( parentHandle );
        SAVE( animFrame );

    }

    void rewindAnimFrame( Entity* entity, float timescale ) {

        auto dAnimFrame = entity->animFrame - old_animFrame;

        if ( dAnimFrame > 0 ) {

            uint16_t framesToAdd = (uint16_t) floorf( dAnimFrame * timescale );

            float lostFrames = dAnimFrame * timescale - framesToAdd;
            float u = (float) rand() / RAND_MAX;
            if ( u < lostFrames )
                framesToAdd++;

            entity->animFrame = old_animFrame + framesToAdd;

        }

    }

    void rewind( EntityRecord* rec, float timescale ) {

        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;

        // Position isn't updated when an entity is mounted,
        // so don't rewind position on unmount.
        if ( entity->parentHandle == old_parentHandle )
            REWIND_VEC( pos, Vec3 );

        REWIND_VEC( vel, Vec3 );

        rewindAnimFrame( entity, timescale );

    }

}
