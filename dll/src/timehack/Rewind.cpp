#include "./headers/Rewind.h"
#include "../headers/Halo1.h"
#include "../utils/headers/common.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

using namespace Halo1;

#define REWIND_VEC(field, type) \
    { type delta_##field = Vec::sub(entity->field, old_##field); \
    entity->field = Vec::add(old_##field, Vec::scale(delta_##field, timescale)); }

#define REWIND_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - old_##field; \
    entity->field = old_##field + (type)(delta_##field * timescale); }

#define REWIND_INCREASES_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - old_##field; \
    if (delta_##field > (type) 0) \
        entity->field = old_##field + (type)(delta_##field * timescale); }

#define REWIND_DECREASES_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - old_##field; \
    if (delta_##field < (type) 0) \
        entity->field = old_##field + (type)(delta_##field * timescale); }

#define REWIND(field, type) REWIND_WITH_TIMESCALE(field, type, timescale)
#define REWIND_INCREASES(field, type) REWIND_INCREASES_WITH_TIMESCALE(field, type, timescale)
#define REWIND_DECREASES(field, type) REWIND_DECREASES_WITH_TIMESCALE(field, type, timescale)

#define SAVE(field) old_##field = entity->field

namespace Rewind {

    // Fields
    Vec3 old_pos, old_vel, old_fwd, old_up, old_angularVelocity;
    float old_fuse, old_heat, old_projectileAge, old_shield;
    uint32_t old_parentHandle, old_ageMilis;
    uint16_t old_animFrame;

    void snapshot( EntityRecord* rec ) {

        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;

        SAVE( pos );
        SAVE( vel );
        SAVE( parentHandle );
        SAVE( animFrame );
        SAVE( ageMilis );

        switch ( (EntityCategory) entity->entityCategory ) {
            case EntityCategory_Biped: {
                SAVE( shield );
                break;
            }
            case EntityCategory_Weapon: {
                SAVE( heat );
                break;
            }
            case EntityCategory_Projectile: {
                SAVE( projectileAge );
                SAVE( fuse );
                break;
            }
            case EntityCategory_Vehicle: {
                SAVE( angularVelocity );
                SAVE( fwd );
                SAVE( up );
                break;
            }
            default: {
                break;
            }
        }

    }

    void rewindAnimFrame( Entity* entity, float timescale );
    void rewindRotation( Entity* entity, float timescale );

    void rewind( EntityRecord* rec, float timescale, float globalTimescale ) {

        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;

        // Position isn't updated when an entity is mounted, so don't rewind position on unmount.
        if ( entity->parentHandle == old_parentHandle )
            REWIND_VEC( pos, Vec3 );

        REWIND_VEC( vel, Vec3 );

        rewindAnimFrame( entity, timescale );
        REWIND( ageMilis, uint32_t );

        switch ( (EntityCategory) entity->entityCategory ) {
            case EntityCategory_Biped: {
                // Shields shouldn't update when player isn't moving.
                REWIND_INCREASES_WITH_TIMESCALE( shield, float, globalTimescale );
                break;
            }
            case EntityCategory_Weapon: {
                // Weapon heat shouldn't update when player isn't moving.
                REWIND_DECREASES_WITH_TIMESCALE( heat, float, globalTimescale );
                break;
            }
            case EntityCategory_Projectile: {
                REWIND( projectileAge, float );
                REWIND( fuse, float );
                break;
            }
            case EntityCategory_Vehicle: {
                REWIND_VEC( angularVelocity, Vec3 );
                rewindRotation( entity, timescale );
                break;
            }
            default: {
                break;
            }
        }

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

    void rewindRotation( Entity* entity, float timescale ) {
        entity->fwd = Vec::unit( Vec::lerp( old_fwd, entity->fwd, timescale ) );
        entity->up = Vec::unit(
            Vec::rejection(
                Vec::lerp( old_up, entity->up, timescale ),
                entity->fwd
            )
        );
    }

}
