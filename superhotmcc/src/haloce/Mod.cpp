#include "Mod.hpp"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "asmjit/x86.h"
#include "MinHook.h"
#include "utils/Utils.hpp"
#include "memory/Allocation.hpp"
#include "asm/Hook.hpp"
#include "asm/AsmHelper.hpp"
#include "Halo1.hpp"
#include "Rewind.hpp"

namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

    uintptr_t halo1 = 0;

    uint64_t updateCount = 0;

    // Associates bone interpolation state with an entity.
    struct AnimationState {
        uint16_t animId;
        uint16_t frame;
        float frameProgress = 0;
        uint64_t lastUpdate = 0; 
    };
    // Bones pointer to animation state.
    std::unordered_map<uint64_t, AnimationState> animationStates;

    AnimationState* getAnimationState( Halo1::Entity* entity ) {
        auto bones = entity->getBoneTransforms();
        if (!bones) return nullptr;
        auto bonesPtr = (uint64_t) bones;
        if ( !animationStates.count( bonesPtr ) )
            animationStates[bonesPtr] = AnimationState{};
        return &animationStates[bonesPtr];
    }
    
    namespace FunctionHooks {

        typedef void (*updateAllEntities_t)( void );
        updateAllEntities_t originalUpdateAllEntities = nullptr;
        //
        void hkUpdateAllEntities() {
            originalUpdateAllEntities();

            // Clear out stale animation states.
            std::vector<uint64_t> staleStates;
            for (auto& [bonesPtr, state] : animationStates) {
                if (updateCount - state.lastUpdate > 10)
                    staleStates.push_back( bonesPtr );
            }
            for (auto bonesPtr : staleStates) {
                // std::cout << "Clearing stale animation state for bones at " << (void*) bonesPtr << std::endl;
                animationStates.erase( bonesPtr );
            }
            // if (updateCount % 120 == 0)
            //     std::cout << "Animation states: " << animationStates.size() << std::endl;

            updateCount++;
        }
        
        typedef uint64_t (*updateEntity_t)( uint32_t entityHandle );
        updateEntity_t originalUpdateEntity = nullptr;
        //
        uint64_t hkUpdateEntity( uint32_t entityHandle ) {

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
            float globalTimeScale = settings.timeScale;
            float timeScale = globalTimeScale;
            if (
                rec->typeId == Halo1::TypeID_Player ||
                Halo1::isRidingTransport( entity ) ||
                Halo1::isTransport( entity )
            )
                timeScale = 1.0f;

            // Update animation progress for bipeds.
            AnimationState* animState = nullptr;
            if (entity->entityCategory == Halo1::EntityCategory_Biped) {
                animState = getAnimationState( entity );
                if (animState) {
                    animState->animId = entity->animId;
                    animState->frame = entity->animFrame;
                    animState->frameProgress += timeScale;
                    animState->lastUpdate = updateCount;
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
                bone.translation = Vec3::lerp(snap.translation, bone.translation, progress);
                bone.scale = snap.scale + (bone.scale - snap.scale) * progress;
            }
        }

        void init() {
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
        }

        void free() {
            MH_DisableHook( (void*) originalUpdateEntity );
            MH_DisableHook( (void*) originalAnimateBones );
            MH_DisableHook( (void*) originalUpdateAllEntities );
        }

    }

    // This may be going away. We only really need to hook the updateEntity function.
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

    void printInitDebugInfo();

    void init() {
        const std::string moduleName = "halo1.dll";
        halo1 = (uintptr_t) Utils::waitForModule(moduleName);
        std::cout << moduleName << ": " << (void*) halo1 << std::endl;

        Halo1::init();

        FunctionHooks::init();
        // JumpHooks::init();

        printInitDebugInfo();
    }

    void free() {
        FunctionHooks::free();
        // JumpHooks::free();
    }

    void printInitDebugInfo() {
        // std::cout << "Entity.entityCategory offset: " << (void*) offsetof( Halo1::Entity, entityCategory ) << std::endl;

        // std::cout << "pos offset: " << (void*) offsetof( Halo1::Camera, pos ) << std::endl;
        // std::cout << "fov offset: " << (void*) offsetof( Halo1::Camera, fov ) << std::endl;
        // std::cout << "fwd offset: " << (void*) offsetof( Halo1::Camera, fwd ) << std::endl;
        // std::cout << "up offset: " << (void*) offsetof( Halo1::Camera, up ) << std::endl;

        // std::cout << "animFrame offset: " << (void*) offsetof( Halo1::Entity, animFrame ) << std::endl;
    }

    void printDebugInfo() {
        // Print player handle
        // std::cout << "Player handle: " << (void*) Halo1::getPlayerHandle() << std::endl;

        // // Print map header pointer.
        // auto mapHeader = Halo1::getMapHeader();
        // std::cout << "Map header: " << (void*) mapHeader << std::endl;
        // if ( mapHeader ) {
        //     std::cout << "Map name at: " << (void*) mapHeader->mapName << std::endl;
        //     std::cout << "Map name: " << mapHeader->mapName << std::endl;
        // }
    }

    void modThreadUpdate() {
        if ( GetAsyncKeyState( VK_F1 ) & 1 ) {
            printDebugInfo();
        }
    }

}