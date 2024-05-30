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

namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

    uintptr_t halo1 = 0;

    namespace FunctionHooks {
        typedef uint64_t (*updateEntity_t)( uint32_t entityHandle );
        updateEntity_t oUpdateEntity = nullptr;
        uint64_t hkUpdateEntity( uint32_t entityHandle ) {
            auto rec = Halo1::getEntityRecord( entityHandle );
            if (!rec) return oUpdateEntity( entityHandle);
            auto entity = rec->entity();
            if (!entity) return oUpdateEntity( entityHandle );
            Halo1::EntityCategory cat = (Halo1::EntityCategory) entity->entityCategory;
            if (cat == Halo1::EntityCategory_Biped) {
                return 1; // No updates for bipeds.
            }
            return oUpdateEntity( entityHandle );
        }

        void init() {
            void* pUpdateEntity = (void*) (halo1 + 0xB3A06CU);
            std::cout << "UpdateEntity: " << pUpdateEntity << std::endl;
            MH_CreateHook( pUpdateEntity, hkUpdateEntity, (void**) &oUpdateEntity );
            MH_EnableHook( pUpdateEntity );
        }

        void free() {
            MH_DisableHook( (void*) oUpdateEntity );
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

    void printExtraDebugInfo();

    void init() {
        const std::string moduleName = "halo1.dll";
        halo1 = (uintptr_t) Utils::waitForModule(moduleName);
        std::cout << moduleName << ": " << (void*) halo1 << std::endl;

        Halo1::init();

        FunctionHooks::init();
        JumpHooks::init();

        printExtraDebugInfo();
    }

    void free() {
        FunctionHooks::free();
        JumpHooks::free();
    }

    void printExtraDebugInfo() {
        // Print offset of Entity.entityCategory
        std::cout << "Entity.entityCategory offset: " << (void*) offsetof( Halo1::Entity, entityCategory ) << std::endl;
    }

    void modThreadUpdate() {
        if ( GetAsyncKeyState( VK_F1 ) & 1 ) {

            // Print player handle
            std::cout << "Player handle: " << (void*) Halo1::getPlayerHandle() << std::endl;

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
    }

}