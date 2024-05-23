#pragma once

#include "asmjit/x86.h"
#include <Windows.h>

class Hook {

    public:
        asmjit::x86::Assembler m_assembler;

        Hook(
            uintptr_t targetAddress, 
            size_t instructionSize, 
            uint8_t*& trampolineMem,
            bool dryRun = false
        );
        ~Hook();

        void install();
        void uninstall();
    
    private:
        asmjit::CodeHolder m_codeHolder;
        asmjit::JitRuntime m_rt;
        asmjit::Environment m_env;

        uintptr_t m_targetAddress;
        size_t m_instructionSize;
        uint8_t*& m_trampolineMem;
        bool m_dryRun;

        uint8_t m_originalBytes[256];
};