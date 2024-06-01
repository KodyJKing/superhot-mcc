#include <Windows.h>
#include <iostream>
#include <vector>
#include "asmjit/x86.h"
#include "MinHook.h"
#include "utils/Utils.hpp"
#include "memory/Allocation.hpp"
#include "asm/Hook.hpp"
#include "asm/AsmHelper.hpp"
#include "Mod.hpp"
#include "Halo1.hpp"
#include "Rewind.hpp"

namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

    uintptr_t halo1 = 0;

    namespace FunctionHooks {
        
        typedef uint64_t (*updateEntity_t)( uint32_t entityHandle );

        updateEntity_t originalUpdateEntity = nullptr;

        uint64_t hkUpdateEntity( uint32_t entityHandle ) {

            auto rec = Halo1::getEntityRecord( entityHandle );
            if (!rec) 
                return originalUpdateEntity( entityHandle);
            auto entity = rec->entity();
            if (!entity) 
                return originalUpdateEntity( entityHandle );

            // Snapshot the entity.
            Halo1::Entity snap = *entity;

            uint64_t result = originalUpdateEntity( entityHandle );

            // Internpolate old and new entity states according to timescale.
            float globalTimeScale = 0.25f;
            float timeScale = globalTimeScale;
            if (
                rec->typeId == Halo1::TypeID_Player ||
                Halo1::isRidingTransport( entity ) ||
                Halo1::isTransport( entity )
            )
                timeScale = 1.0f;
            Rewind::rewind( rec, timeScale, globalTimeScale, snap );


            return result;
        }

        void init() {
            void* pUpdateEntity = (void*) (halo1 + 0xB3A06CU);
            std::cout << "UpdateEntity: " << pUpdateEntity << std::endl;
            MH_CreateHook( pUpdateEntity, hkUpdateEntity, (void**) &originalUpdateEntity );
            MH_EnableHook( pUpdateEntity );
        }

        void free() {
            MH_DisableHook( (void*) originalUpdateEntity );
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

        std::cout << "animFrame offset: " << (void*) offsetof( Halo1::Entity, animFrame ) << std::endl;
    }

    void printDebugInfo() {
        // Print player camera pointer.
        // std::cout << "Player camera: " << (void*) Halo1::getPlayerCameraPointer() << std::endl;

        // Print player handle
        // std::cout << "Player handle: " << (void*) Halo1::getPlayerHandle() << std::endl;

        // // Print isGameLoaded.
        // std::cout << "isGameLoaded: " << Halo1::isGameLoaded() << std::endl;

        // Print all entity pointers.
        // std::cout << std::endl;
        // Halo1::foreachEntityRecord( []( Halo1::EntityRecord* entityRecord ) {
        //     if ( !entityRecord ) 
        //         return true;

        //     Halo1::Entity* entity = entityRecord->entity();
        //     std::cout << "Entity: " << (void*) entity << std::endl;

        //     // Print entity tag resource path.
        //     if (entity) {
        //         char* resourcePath = entity->getTagResourcePath();
        //         if ( resourcePath )
        //             std::cout << "Tag: " << resourcePath << std::endl;
        //     }

        //     std::cout << std::endl;

        //     return true;
        // });

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