#include "Mod.hpp"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "asmjit/x86.h"
#include "MinHook.h"
#include "utils/Utils.hpp"
#include "utils/UnloadLock.hpp"
#include "memory/Memory.hpp"
#include "memory/Allocation.hpp"
#include "asm/Hook.hpp"
#include "asm/AsmHelper.hpp"
#include "Halo1.hpp"
#include "Rewind.hpp"
#include "TimeScale.hpp"

namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

    // Deadzoning is intended to prevent discrete actions (like spawning projectiles) from being spammed when an entity should be nearly frozen.
    float timescaleUpdateDeadzone = 0.05f;

    float playerDamageMultiplier = 3.0f;
    float npcDamageMultiplier = 2.0f;

    uintptr_t halo1 = 0;

    // Data associated with entity for pose interpolation.
    struct AnimationState {
        uint16_t animId;
        uint16_t frame;
        float frameProgress = 0;
        uint64_t lastUpdateTick = 0; 
    };
    // Bones pointer to animation state.
    std::unordered_map<uint64_t, AnimationState> animationStates;
    
    uint64_t tickCount = 0;

    void clearStaleAnimationStates() {
        std::vector<uint64_t> staleStates;
        for (auto& [bonesPtr, state] : animationStates) {
            if (tickCount - state.lastUpdateTick > 10)
                staleStates.push_back( bonesPtr );
        }
        for (auto bonesPtr : staleStates) {
            // std::cout << "Clearing stale animation state for bones at " << (void*) bonesPtr << std::endl;
            animationStates.erase( bonesPtr );
        }
        // if (tickCount % 120 == 0)
        //     std::cout << "Animation states: " << animationStates.size() << std::endl;
    }

    AnimationState* getAnimationState( Halo1::Entity* entity ) {
        auto bones = entity->getBoneTransforms();
        if (!bones) return nullptr;
        auto bonesPtr = (uint64_t) bones;
        if ( !animationStates.count( bonesPtr ) )
            animationStates[bonesPtr] = AnimationState{};
        return &animationStates[bonesPtr];
    }
    
    typedef void (*updateAllEntities_t)( void );
    updateAllEntities_t originalUpdateAllEntities = nullptr;
    //
    void hkUpdateAllEntities() {
        UnloadLock lock; // No unloading while we're still executing hook code.

        TimeScale::update();
        originalUpdateAllEntities();
        clearStaleAnimationStates();
        tickCount++;
    }
    
    typedef uint64_t (*updateEntity_t)( uint32_t entityHandle );
    updateEntity_t originalUpdateEntity = nullptr;
    //
    uint64_t hkUpdateEntity( uint32_t entityHandle ) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec) 
            return originalUpdateEntity( entityHandle);
            
        auto entity = rec->entity();
        if (!entity)  
            return originalUpdateEntity( entityHandle );
            
        if (!settings.enableTimeScale) 
            return originalUpdateEntity( entityHandle );

        // Snapshot the entity.
        Halo1::Entity snap = *entity;

        // Compute timescale.
        float globalTimeScale = TimeScale::timescale; // settings.timeScale;
        float timeScale = globalTimeScale;
        if (
            rec->typeId == Halo1::TypeID_Player ||
            Halo1::isRidingTransport( entity ) ||
            Halo1::isTransport( entity )
        )
            timeScale = 1.0f;
        
        // if (
        //     timeScale < timescaleUpdateDeadzone &&
        //     entity->entityCategory == Halo1::EntityCategory_Weapon
        // )
        //     return 1;

        // Advance animation progress by timescale.
        AnimationState* animState = nullptr;
        if (entity->entityCategory == Halo1::EntityCategory_Biped) {
            animState = getAnimationState( entity );
            if (animState) {
                animState->animId = entity->animId;
                animState->frame = entity->animFrame;
                animState->frameProgress += timeScale;
                animState->lastUpdateTick = tickCount;
            }
        }

        // Do update
        uint64_t result = originalUpdateEntity( entityHandle );

        // Interpolate old and new entity states according to timescale.
        Rewind::rewind( rec, timeScale, globalTimeScale, snap );

        // Advance animation frame once we've progressed through a whole frame.
        if (animState) {
            if (entity->animId != animState->animId) {
                animState->frameProgress = 0;
            } else {
                if (animState->frameProgress >= 1)
                    animState->frameProgress = 0;
                else
                    entity->animFrame = animState->frame;
            }
        }

        return result;
    }

    typedef void (*animateBones_t)(uint64_t param1, void* animation, uint16_t frame, Halo1::Transform* bones);
    animateBones_t originalAnimateBones = nullptr;
    //
    void hkAnimateBones(uint64_t param1, void* animation, uint16_t frame, Halo1::Transform* bones) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        if (
            !settings.enableTimeScale || 
            !settings.poseInterpolation ||
            !animationStates.count((uint64_t) bones)
        )
            return originalAnimateBones(param1, animation, frame, bones);

        auto animState = animationStates[(uint64_t) bones];
        if (animState.frame == frame)
            return originalAnimateBones(param1, animation, frame, bones);

        // Todo: Interpolate poses in transitions between different animations, not just frames.

        // Get initial bone state.
        originalAnimateBones(param1, animation, animState.frame, bones);

        // Save a snapshot.
        uint16_t boneCount = Halo1::boneCount(animation);
        std::vector<Halo1::Transform> boneSnap;
        for (uint16_t i = 0; i < boneCount; i++)
            boneSnap.push_back(bones[i]);

        // Get target bone state.
        originalAnimateBones(param1, animation, frame, bones);

        // Interpolate bone state.
        float progress = animState.frameProgress;
        for ( uint16_t i = 0; i < boneCount; i++ ) {
            auto& bone = bones[i];
            auto& snap = boneSnap[i];
            bone.rotation = snap.rotation.nlerp(bone.rotation, progress);
            bone.translation = snap.translation.lerp(bone.translation, progress);
            bone.scale = snap.scale + (bone.scale - snap.scale) * progress;
        }
    }

    typedef void (*damageEntity_t)(uint32_t entityHandle, uint16_t param_2, uint16_t param_3, uint64_t param_4, uint8_t* param_5, void* param_6, uint64_t param_7, uint64_t param_8, uint32_t* param_9, float* param_10, uint32_t* param_11, float damage, char param_13);
    damageEntity_t originalDamageEntity = nullptr;
    //
    void hkDamageEntity(uint32_t entityHandle, uint16_t param_2, uint16_t param_3, uint64_t param_4, uint8_t* param_5, void* param_6, uint64_t param_7, uint64_t param_8, uint32_t* param_9, float* param_10, uint32_t* param_11, float damage, char param_13) {
        UnloadLock lock; // No unloading while we're still executing hook code.
        
        if (!settings.enableTimeScale)
            return originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);
        
        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec)
            return originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);

        if (rec->typeId == Halo1::TypeID_Player)
            damage *= playerDamageMultiplier;
        else
            damage *= npcDamageMultiplier;

        return originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);
    }

    void hookFunctions() {
        void* pUpdateEntity = (void*) (halo1 + 0xB3A06CU);
        std::cout << "UpdateEntity: " << pUpdateEntity << std::endl;
        MH_CreateHook( pUpdateEntity, hkUpdateEntity, (void**) &originalUpdateEntity );
        MH_EnableHook( pUpdateEntity );

        void* pAnimateBones = (void*) (halo1 + 0xC41984U);
        std::cout << "AnimateBones: " << pAnimateBones << std::endl;
        MH_CreateHook( pAnimateBones, hkAnimateBones, (void**) &originalAnimateBones );
        MH_EnableHook( pAnimateBones );

        void* pUpdateAllEntities = (void*) (halo1 + 0xB35654U);
        std::cout << "UpdateAllEntities: " << pUpdateAllEntities << std::endl;
        MH_CreateHook( pUpdateAllEntities, hkUpdateAllEntities, (void**) &originalUpdateAllEntities );
        MH_EnableHook( pUpdateAllEntities );

        void* pDamageEntity = (void*) (halo1 + 0xB9FBD0U);
        std::cout << "DamageEntity: " << pDamageEntity << std::endl;
        MH_CreateHook( pDamageEntity, hkDamageEntity, (void**) &originalDamageEntity );
        MH_EnableHook( pDamageEntity );
    }

    void unhookFunctions() {
        MH_DisableHook( (void*) originalUpdateEntity );
        MH_DisableHook( (void*) originalAnimateBones );
        MH_DisableHook( (void*) originalUpdateAllEntities );
        MH_DisableHook( (void*) originalDamageEntity );
    }

    std::vector<Memory::PatchPtr> patches;
    void patchTags() {
        // Limit plasma pistol rate of fire.
        const int projTagIndex = 0;
        auto plasmaPistolTag = Halo1::findTag( "weapons\\plasma pistol\\plasma pistol", "weap" );
        std::cout << "Plasma Pistol tag: " << plasmaPistolTag << std::endl;
        auto projData = Halo1::getProjectileData( plasmaPistolTag, projTagIndex );
        std::cout << "Plasma Pistol Projectile data: " << projData << std::endl;
        if (!projData) return;
        patches.push_back( Memory::createPatch( projData->minRateOfFire, 15.0f ) );
        patches.push_back( Memory::createPatch( projData->maxRateOfFire, 15.0f ) );

        // { // Test setting sniper rof really high.
        //     auto assaultRifleTag = Halo1::findTag( "weapons\\assault rifle\\assault rifle", "weap" );
        //     auto assaultProjData = Halo1::getProjectileData( assaultRifleTag, 0 );
        //     if (!assaultProjData) return;
        //     uint32_t assaultFlags = *(uint32_t*) assaultProjData;

        //     auto sniperTag = Halo1::findTag( "weapons\\sniper rifle\\sniper rifle", "weap" );
        //     std::cout << "Sniper Rifle tag: " << sniperTag << std::endl;
        //     auto sniperProjData = Halo1::getProjectileData( sniperTag, 0 );
        //     std::cout << "Sniper Rifle Projectile data: " << sniperProjData << std::endl;
        //     if (!sniperProjData) return;
        //     uint32_t* flags = (uint32_t*) sniperProjData;
        //     patches.push_back( Memory::createPatch( *flags, assaultFlags ) );
        //     patches.push_back( Memory::createPatch( sniperProjData->minRateOfFire, 100.0f ) );
        //     patches.push_back( Memory::createPatch( sniperProjData->maxRateOfFire, 100.0f ) );
        // }
    }
    void unpatchTags() {
        patches.clear();
    }

    void init() {
        const std::string moduleName = "halo1.dll";
        halo1 = (uintptr_t) Utils::waitForModule(moduleName);
        std::cout << moduleName << ": " << (void*) halo1 << std::endl;

        Halo1::init();
        hookFunctions();
        TimeScale::init();

        patchTags();
    }

    void free() {
        unpatchTags();
        unhookFunctions();
    }

    // Called by mod dll's thread regularly.
    void modThreadUpdate() {
    }

    ////////////////////////////////////////////////////////////////////////////

    // Not currently used, but we may use jump hooks in the near future, so let's keep this for reference.
    namespace JumpHooks {
        const size_t codeSize = 0x1000;
        uint8_t* codeMemory = nullptr;

        std::vector<HookPtr> hooks;
        HookPtr addHook( 
            std::string name,
            uintptr_t targetAddress,
            size_t instructionSize
        ) {
            HookPtr hook = std::make_shared<Hook>( name, targetAddress, instructionSize );
            hooks.push_back( hook );
            return hook;
        }

        void onConsumeFrag() {
            std::cout << "Consume frag!" << std::endl;
        }

        void init() {
            codeMemory = (uint8_t*) Memory::virtualAllocateNear( halo1, codeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
            std::cout << "Code: " << (void*)codeMemory << std::endl;

            HookPtr hook = addHook( "ConsumeFrags", halo1 + 0xB0A330, 7 ); {
                x86::Assembler& a = hook->m_assembler;
                // Original code
                // dec byte ptr [rdx+rbx+2FCh]
                a.dec( x86::byte_ptr( x86::rdx, x86::rbx, 0, 0x2FC ) );
                // Our code
                AsmHelper::push( a );
                CALL_0( a, (uintptr_t) onConsumeFrag );
                AsmHelper::pop( a );
            }

            uint8_t* freeCodeMemory = codeMemory;
            for (HookPtr hook : hooks) 
                hook->install(freeCodeMemory);

            DWORD oldProtect;
            VirtualProtect( codeMemory, codeSize, PAGE_EXECUTE_READ, &oldProtect );
        }

        void free() {
            hooks.clear();

            if ( codeMemory ) {
                VirtualFree( codeMemory, 0, MEM_RELEASE );
                codeMemory = nullptr;
            }
        }
    }

}