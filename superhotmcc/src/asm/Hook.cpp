#include "Hook.hpp"
#include "utils/Threads.hpp"
#include <iostream>

#pragma pack(push, 1)
struct Jmp {
    uint8_t jmp = 0xE9;
    int32_t offset;
    Jmp(int32_t offset) : offset(offset) {}
};

Hook::Hook(
    std::string name,
    uintptr_t targetAddress, 
    size_t instructionSize
) : 
    m_name(name),
    m_targetAddress(targetAddress), m_instructionSize(instructionSize), 
    m_env(m_rt.environment()), m_assembler(&m_codeHolder) 
 {
    std::cout << m_name << ": " << (void*) m_targetAddress << " (location)" << std::endl;
    m_codeHolder.init( m_env );
    m_codeHolder.attach( &m_assembler );
 }

void Hook::install(uint8_t*& trampolineMem) {
    // Add return jmp.
    m_assembler.jmp(m_targetAddress + m_instructionSize);

    // Finalize and relocate codeHolder to trampolineMem
    m_assembler.finalize();
    m_codeHolder.relocateToBase( (uint64_t) trampolineMem );
    auto trampolineSize = m_codeHolder.codeSize();
    m_codeHolder.copySectionData( trampolineMem, trampolineSize, 0, asmjit::CopySectionFlags::kPadSectionBuffer | asmjit::CopySectionFlags::kPadTargetBuffer );

    // Update trampolineMem
    uintptr_t trampolineAddress = (uintptr_t) trampolineMem;
    trampolineMem += trampolineSize;

    std::cout << m_name << ": " << (void*) trampolineAddress << " (trampoline)" << std::endl;

    // Save original bytes
    memcpy( m_originalBytes, (void*) m_targetAddress, m_instructionSize );

    // Point targetAddress to hook
    if (m_dryRun) return;
    Threads::freezeOthersEx(m_targetAddress, m_targetAddress + m_instructionSize);
    DWORD oldProtect;
    VirtualProtect( (void*) m_targetAddress, m_instructionSize, PAGE_EXECUTE_READWRITE, &oldProtect );
    memset( (void*) m_targetAddress, 0x90, m_instructionSize ); // NOPs
    Jmp instruction( (int32_t) (trampolineAddress - m_targetAddress - 5) );
    memcpy( (void*) m_targetAddress, &instruction, 5 );
    VirtualProtect( (void*) m_targetAddress, m_instructionSize, oldProtect, &oldProtect );
    Threads::unfreeze();
}

void Hook::uninstall() {
    Threads::freezeOthersEx(m_targetAddress, m_targetAddress + m_instructionSize);
    DWORD oldProtect;
    VirtualProtect( (void*) m_targetAddress, m_instructionSize, PAGE_EXECUTE_READWRITE, &oldProtect );
    memcpy( (void*) m_targetAddress, m_originalBytes, m_instructionSize );
    VirtualProtect( (void*) m_targetAddress, m_instructionSize, oldProtect, &oldProtect );
    Threads::unfreeze();
}

Hook::~Hook() {
    uninstall();
}