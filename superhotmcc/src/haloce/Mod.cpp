#include <Windows.h>
#include <iostream>
#include <vector>
#include "asmjit/x86.h"
#include "utils/Utils.hpp"
#include "memory/Allocation.hpp"
#include "asm/Hook.hpp"
#include "asm/AsmHelper.hpp"
#include "Mod.hpp"
#include "Halo1.hpp"

namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

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
        const std::string moduleName = "halo1.dll";
        uintptr_t halo1 = (uintptr_t) Utils::waitForModule(moduleName);
        std::cout << moduleName << ": " << (void*) halo1 << std::endl;

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

        Halo1::init();
    }

    void free() {
        hooks.clear();
        if ( codeMemory ) {
            VirtualFree( codeMemory, 0, MEM_RELEASE );
            codeMemory = nullptr;
        }
    }

    void modThreadUpdate() {
        if ( GetAsyncKeyState( VK_F1 ) & 1 ) {

            Halo1::foreachEntityRecord( []( Halo1::EntityRecord* entityRecord ) {
                if ( !entityRecord ) return true;
                Halo1::Entity* entity = entityRecord->entity();
                std::cout << "Entity: " << (void*) entity << std::endl;
                return true;
            });

        }
    }

}