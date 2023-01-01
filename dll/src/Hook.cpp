#include "./headers/Hook.h"
#include "./headers/AllocationUtils.h"

namespace Hook {

    // Byte codes for push rax, push rbx ... push r15.
    char pusha64[] = { '\x50', '\x53', '\x51', '\x52', '\x56', '\x57', '\x55', '\x54', '\x41', '\x50', '\x41', '\x51', '\x41', '\x52', '\x41', '\x53', '\x41', '\x54', '\x41', '\x55', '\x41', '\x56', '\x41', '\x57' };
    // Byte codes for pop r15 ... push rbx, pop rax.
    char popa64[]  = { '\x41', '\x5F', '\x41', '\x5E', '\x41', '\x5D', '\x41', '\x5C', '\x41', '\x5B', '\x41', '\x5A', '\x41', '\x59', '\x41', '\x58', '\x5C', '\x5D', '\x5F', '\x5E', '\x5A', '\x59', '\x5B', '\x58' };

    // === Buffer Writing ===
    inline void writeBytes(char** pDest, char* src, size_t count) {
        memcpy(*pDest, src, count);
        *pDest += count;
    }

    template<typename T>
    inline void write(char** pDest, T content) {
        writeBytes(pDest, (char*) &content, sizeof(T));
    }

    inline void writeOffset(char** pDest, UINT_PTR address) {
        INT_PTR offset = (INT_PTR) address - (INT_PTR)*pDest - (INT_PTR) sizeof(DWORD);
        int32_t i32Offset = (int32_t) offset;
        if (i32Offset != offset)
            throw std::runtime_error("Offset does not fit in 32-bit integer.");
        write(pDest, i32Offset);
    }
    // ======================
    
    // === Jump Hook ========
    void JumpHook::allocTrampoline(size_t size) {
        trampolineSize = size;
        // trampolineBytes = (char*) VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
        trampolineBytes = (char*) AllocationUtils::VirtualAllocNear(address, size, MEM_COMMIT, PAGE_READWRITE);
        std::cout << "Allocating trampoline at " << std::uppercase << std::hex << (UINT_PTR) trampolineBytes << std::endl;
    }

    void JumpHook::protectTrampoline() {
        DWORD oldProtect;
        VirtualProtect(trampolineBytes, trampolineSize, PAGE_EXECUTE_READ, &oldProtect);
    }

    void JumpHook::unprotectTrampoline() {
        DWORD oldProtect;
        VirtualProtect(trampolineBytes, trampolineSize, PAGE_EXECUTE_READWRITE, &oldProtect);
    }

    void JumpHook::hook() {
        std::cout << "Adding jump hook: " << description << std::endl;

        stolenBytes = (char*) malloc(numStolenBytes);
        memcpy(stolenBytes, (char*)address, numStolenBytes);

        DWORD oldProtect;
        VirtualProtect((void*)address, numStolenBytes, PAGE_EXECUTE_READWRITE, &oldProtect);

        // Make sure anything not overwritten by jump is a nop
        memset((void*)address, NOP, numStolenBytes);
        char* head = (char*)address;

        // Jump to trampoline
        write(&head, JMP);
        writeOffset(&head, (UINT_PTR) trampolineBytes);

        VirtualProtect((void*)address, numStolenBytes, oldProtect, &oldProtect);
    }

     void JumpHook::freeTrampoline() {
        if (trampolineBytes)
            VirtualFree(trampolineBytes, 0, MEM_RELEASE);
        if (stolenBytes)
            free(stolenBytes);
     }

     void JumpHook::restoreStolenBytes() {
        DWORD oldProtect;
        VirtualProtect((void*)address, numStolenBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)address, stolenBytes, numStolenBytes);
        VirtualProtect((void*)address, numStolenBytes, oldProtect, &oldProtect);
     }

    void JumpHook::unhook() {
        std::cout << "Removing jump hook: " << description << std::endl;
        restoreStolenBytes();
        freeTrampoline();
    }

    // Trampoline code generation

    void JumpHook::writeStolenBytes(char** head) {
        transplantedBytes = *head;
        auto iTransplantOffset = (INT_PTR) address - (INT_PTR) transplantedBytes;
        transplantOffset = (int32_t) iTransplantOffset;
        if (transplantOffset != iTransplantOffset) {
            std::cout << "Transplanted offset: " << std::uppercase << std::hex << iTransplantOffset << std::endl;
            throw std::runtime_error("Transplanted offset does not fit in 32-bit integer.");
        }
        writeBytes(head, (char*)address, numStolenBytes);
    }

    void JumpHook::fixStolenOffset(size_t stolenByteIndex) {
        char* pOffset = transplantedBytes + stolenByteIndex;
        int* pOffsetI = (int*) pOffset;
        *pOffsetI += transplantOffset;
    }

    void JumpHook::writeReturnJump(char** head) {
        write(head, JMP);
        writeOffset(head, address + numStolenBytes);
    }

    void JumpHook::writePushState(char** head) {
        write(head, PUSHF);
        #ifndef _WIN64
            write(head, PUSHA);
        #else
            writeBytes(head, pusha64, ARRAYSIZE(pusha64));
        #endif
    }

    void JumpHook::writePopState(char** head) {
        #ifndef _WIN64
            write(head, POPA);
        #else
            writeBytes(head, popa64, ARRAYSIZE(popa64));
        #endif
        write(head, POPF);
    }

    void JumpHook::writeJump(char** head, UINT_PTR hookFunc) {
        write(head, JMP);
        writeOffset(head, hookFunc);
        trampolineReturn = (UINT_PTR) *head;
    }

    void JumpHook::writeCall(char** head, UINT_PTR hookFunc) {
        write(head, CALL);
        writeOffset(head, hookFunc);
    }
    // ======================

    std::vector<JumpHook> jumpHookRecords;

    JumpHook addJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags,
        UINT_PTR* returnAddress
    ) {
        JumpHook record = { description, address, numStolenBytes };

        try {
            record.allocTrampoline(numStolenBytes + 1000);

            // === Write trampoline code ===
            char* head = record.trampolineBytes;

            bool execStolenAfter = flags & HK_STOLEN_AFTER;

            if (!execStolenAfter)
                record.writeStolenBytes(&head);

            if (flags & HK_PUSH_STATE)
                record.writePushState(&head);
            
            if (flags & HK_JUMP)
                record.writeJump(&head, hookFunc);
            else
                record.writeCall(&head, hookFunc);

            if (flags & HK_PUSH_STATE)
                record.writePopState(&head);

            if (execStolenAfter)
                record.writeStolenBytes(&head);

            record.writeReturnJump(&head);

            if (returnAddress != nullptr)
                *returnAddress = record.trampolineReturn;
            // =============================
        }
        catch ( std::exception &e ) {
            std::cout << "An error occured while trying to create trampoline: " << e.what() << std::endl;
            record.freeTrampoline();
            return record;
        }

        try {
            record.protectTrampoline();
            record.hook();

            std::cout << "Trampoline located at: " << std::hex << (UINT_PTR)record.trampolineBytes << std::endl << std::endl;

            jumpHookRecords.push_back(record);
            return record;

        } 
        catch ( std::exception &e ) {
            std::cout << "An error occured while trying to hook function: " << e.what() << std::endl;
            record.unhook();
            return record;
        }

    }

    JumpHook addJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags
    ) {
        return addJumpHook(description, address, numStolenBytes, hookFunc, flags, nullptr);
    }

    void removeAllJumpHookRecords() {
        for (uint64_t i = 0; i < jumpHookRecords.size(); i++)
            jumpHookRecords[i].unhook();
    }

}
