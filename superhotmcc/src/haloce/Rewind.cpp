#include "Halo1.hpp"
#include <iostream>

using namespace Halo1;

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
    static Vec3 old_pos, old_vel, old_fwd, old_up, old_angularVelocity;
    static float old_fuse, old_heat, old_projectileAge, old_shield;
    static uint32_t old_parentHandle, old_ageMilis;
    static uint16_t old_animFrame;
    static uint8_t old_ticksSinceLastFired;

    // struct Snapshot {
    //     Vec3 pos, vel, fwd, up, angularVelocity;
    //     float fuse, heat, projectileAge, shield;
    //     uint32_t parentHandle, ageMilis;
    //     uint16_t animFrame;
    //     uint8_t ticksSinceLastFired;
    // };

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
                SAVE( ticksSinceLastFired );
                SAVE( heat );
                break;
            }
            case EntityCategory_Projectile: {
                SAVE( projectileAge );
                SAVE( fuse );
                SAVE( fwd );
                SAVE( up );
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
    void rewindFireCooldown( Entity* entity, float timescale );

    void rewind( EntityRecord* rec, float timescale, float globalTimescale ) {
            
        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;
        
        // Position isn't updated when an entity is mounted, so don't rewind position on unmount.
        if ( entity->parentHandle == old_parentHandle )
            REWIND( pos, Vec3 );

        REWIND( vel, Vec3 );

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
                rewindFireCooldown( entity, globalTimescale );
                break;
            }
            case EntityCategory_Projectile: {
                REWIND( projectileAge, float );
                REWIND( fuse, float );
                rewindRotation( entity, timescale );

                // Do not allow these projectiles to accelerate / decelerate.
                if (
                    entity->fromResourcePath( "vehicles\\warthog\\bullet" ) ||
                    entity->fromResourcePath( "weapons\\pistol\\bullet" ) ||
                    entity->fromResourcePath( "weapons\\plasma rifle\\bolt" ) ||
                    entity->fromResourcePath( "vehicles\\scorpion\\bullet" )
                    )
                    entity->vel = old_vel;

                break;
            }
            case EntityCategory_Vehicle: {
                REWIND( angularVelocity, Vec3 );
                rewindRotation( entity, timescale );
                break;
            }
            default: {
                break;
            }
        }

    }

    void rewindFireCooldown( Entity* entity, float timescale ) {
        static const float timescaleThreshold = 0.6f;
        if ( timescale > timescaleThreshold )
            return;
        auto diff = entity->ticksSinceLastFired - old_ticksSinceLastFired;
        if ( diff == 1 ) {
            entity->ticksSinceLastFired = old_ticksSinceLastFired;
            float u = (float) rand() / RAND_MAX;
            if ( u < timescale )
                entity->ticksSinceLastFired++;
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
        entity->fwd = Vec3::lerp( old_fwd, entity->fwd, timescale ).normalize();
        entity->up = Vec3::lerp( old_up, entity->up, timescale ).rejection(entity->fwd).normalize();
    }

}
