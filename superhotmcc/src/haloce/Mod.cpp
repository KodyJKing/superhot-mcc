#include <Windows.h>
#include <iostream>
#include <vector>
#include "asmjit/x86.h"
#include "memory/Allocation.hpp"
#include "asm/Hook.hpp"
#include "asm/AsmHelper.hpp"


namespace HaloCE::Mod {

    namespace x86 = asmjit::x86;

    std::vector<Hook*> hooks;

    const size_t codeSize = 0x1000;
    uint8_t* codeMemory = nullptr;
    uint8_t* freeCodeMemory = nullptr;

    void onConsumeFrag() {
        std::cout << "Consume frag!" << std::endl;
    }

    void init() {
        HMODULE halo = GetModuleHandleA("halo1.dll");
        uintptr_t haloBase = (uintptr_t) halo;
        std::cout << "halo1.dll: " << halo << std::endl;

        // Allocate memory for code:
        codeMemory = (uint8_t*) Memory::virtualAllocateNear( haloBase, codeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
        freeCodeMemory = codeMemory;
        std::cout << "Code: " << (void*)codeMemory << std::endl;

        // Consume frags hook:
        uintptr_t consumeFrags = haloBase + 0xB0A330;
        std::cout << "ConsumeFrags: " << (void*)consumeFrags << std::endl;
        auto hook = new Hook( consumeFrags, 7, freeCodeMemory );
        asmjit::x86::Assembler& a = hook->m_assembler;
        {
            // Original code
            a.dec( x86::byte_ptr( x86::rdx, x86::rbx, 0, 0x2FC ) );
            // Our code
            AsmHelper::push( a );
            CALL_0( a, (uintptr_t) onConsumeFrag );
            AsmHelper::pop( a );
        }
        hook->install();
        hooks.push_back( hook );

        // Make code read-only
        DWORD oldProtect;
        VirtualProtect( codeMemory, codeSize, PAGE_EXECUTE_READ, &oldProtect );
    }

    void free() {
        for (Hook* hook : hooks) 
            delete hook;
        hooks.clear();

        if ( codeMemory ) {
            VirtualFree( codeMemory, 0, MEM_RELEASE );
            codeMemory = nullptr;
            std::cout << "Code memory freed" << std::endl;
        }
    }

}