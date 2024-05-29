#pragma once

#include "asmjit/x86.h"
#include <Windows.h>
#include <string>
#include <memory>

class Hook {

    public:
        std::string m_name;
        asmjit::x86::Assembler m_assembler;
        bool m_dryRun = false;

        Hook(
            std::string name,
            uintptr_t targetAddress, 
            size_t instructionSize
        );
        ~Hook();

        void install(uint8_t*& trampolineMem);
        void uninstall();
    
    private:
        asmjit::CodeHolder m_codeHolder;
        asmjit::JitRuntime m_rt;
        asmjit::Environment m_env;

        uintptr_t m_targetAddress;
        size_t m_instructionSize;

        uint8_t m_originalBytes[256];
};

using HookPtr = std::shared_ptr<Hook>;