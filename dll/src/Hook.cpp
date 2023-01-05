#include "./headers/Hook.h"
#include "./headers/AllocationUtils.h"
#include "./utils/headers/MathUtils.h"

bool isValid32BitOffset(UINT_PTR offset) {
    return offset >= MININT32 && offset <= MAXINT32;
}

namespace Hook {

    // Byte codes for push rax, push rbx ... push r15.
    char pusha64[] = { '\x50', '\x53', '\x51', '\x52', '\x56', '\x57', '\x55', '\x54', '\x41', '\x50', '\x41', '\x51', '\x41', '\x52', '\x41', '\x53', '\x41', '\x54', '\x41', '\x55', '\x41', '\x56', '\x41', '\x57' };
    // Byte codes for pop r15 ... push rbx, pop rax.
    char popa64[]  = { '\x41', '\x5F', '\x41', '\x5E', '\x41', '\x5D', '\x41', '\x5C', '\x41', '\x5B', '\x41', '\x5A', '\x41', '\x59', '\x41', '\x58', '\x5C', '\x5D', '\x5F', '\x5E', '\x5A', '\x59', '\x5B', '\x58' };
    // sub rsp, 20h
    char allocShadow[] =   { '\x48', '\x83', '\xEC', '\x20' };
    // add rsp, 20h
    char deallocShadow[] = { '\x48', '\x83', '\xC4', '\x20' };

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
        INT_PTR offset = MathUtils::signedDifference(address - sizeof(DWORD), (INT_PTR)*pDest);
        if ( isValid32BitOffset(offset) )
            throw std::runtime_error("Offset does not fit in 32-bit integer.");
        write(pDest, (int32_t) offset);
    }
    // ======================
    
    // === Jump Hook ========
    void JumpHook::allocTrampoline(size_t size) {
        /*
            TODO: Write a custom allocator to avoid wasting a memory.
                  Currently we have to allocate 64KB at a time due to 
                  windows' allocation granularity limit.
        */
        trampolineSize = size;
        trampolineBytes = (char*) AllocationUtils::virtualAllocNear(address, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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
        std::cout << "Trampoline located at: " << std::hex << (UINT_PTR)trampolineBytes << std::endl << std::endl;

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

    void JumpHook::release() {
        this->unhook();
        delete this;
    }

    // Trampoline code generation

    void JumpHook::writeStolenBytes(char** head) {
        transplantedBytes = *head;
        auto transplantOffset64 = (INT_PTR) address - (INT_PTR) transplantedBytes;
        if ( isValid32BitOffset(transplantOffset64) )
            throw std::runtime_error("Transplanted offset does not fit in 32-bit integer.");
        transplantOffset = (int32_t) transplantOffset64;
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
        #ifdef _WIN64
            this->writeAbsoluteJump(head, hookFunc);
        #else
            write(head, JMP);
            writeOffset(head, hookFunc);
        #endif
        trampolineReturn = (UINT_PTR) *head;
    }

    void JumpHook::writeCall(char** head, UINT_PTR hookFunc) {
        #ifdef _WIN64
            writeBytes(head, allocShadow, ARRAYSIZE(allocShadow));
            this->writeAbsoluteCall(head, hookFunc);
            writeBytes(head, deallocShadow, ARRAYSIZE(deallocShadow));
        #else
            write(head, CALL);
            writeOffset(head, hookFunc);
        #endif
    }

    void JumpHook::writeAbsoluteJump(char** head, UINT_PTR hookFunc) {
        write(head, '\xFF');
        write(head, '\x25');
        write(head, (DWORD) 0);
        write(head, hookFunc);
    }

    void JumpHook::writeAbsoluteCall(char** head, UINT_PTR hookFunc) {
        write(head, '\xFF');
        write(head, '\x15');
        write(head, (DWORD) 2);
        write(head, '\xEB');
        write(head, '\x08');
        write(head, hookFunc);
    }

    // ======================

    JumpHook* ezCreateJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags
    ) {
        JumpHook* hook = new JumpHook();
        hook->description = description;
        hook->address = address;
        hook->numStolenBytes = numStolenBytes;

        try {
            hook->allocTrampoline(numStolenBytes + 1000);

            char* head = hook->trampolineBytes;

            bool execStolenAfter = flags & HK_STOLEN_AFTER;

            if (!execStolenAfter)
                hook->writeStolenBytes(&head);

            if (flags & HK_PUSH_STATE)
                hook->writePushState(&head);
            
            if (flags & HK_JUMP)
                hook->writeJump(&head, hookFunc);
            else
                hook->writeCall(&head, hookFunc);

            if (flags & HK_PUSH_STATE)
                hook->writePopState(&head);

            if (execStolenAfter)
                hook->writeStolenBytes(&head);

            hook->writeReturnJump(&head);

        } catch ( std::exception &e ) {
            std::cout << "An error occured while trying to create trampoline: " << e.what() << std::endl;
            hook->freeTrampoline();
            delete hook;
            return nullptr;
        }

        return hook;

    }

    JumpHook* addJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags
    ) {

        JumpHook* hook = ezCreateJumpHook(description, address, numStolenBytes, hookFunc, flags);

        if (!hook) 
            return nullptr;

        try {
            hook->protectTrampoline();
            hook->hook();
        }  catch ( std::exception &e ) {
            std::cout << "An error occured while trying to hook function: " << e.what() << std::endl;
            hook->unhook();
            delete hook;
            return nullptr;
        }

        if ( !( flags & HK_TEMPORARY ) )
            removeBeforeClosing(hook);

        return hook;

    }
    
    // ==============================================

    std::vector<JumpHook*> jumpHooks;

    JumpHook* removeBeforeClosing(JumpHook* hook) {
        jumpHooks.emplace_back(hook);
        return hook;
    }

    void cleanupHooks() {
        // for (uint64_t i = 0; i < jumpHooks.size(); i++)
        //     jumpHooks[i]->release();

        for (uint64_t i = 0; i < jumpHooks.size(); i++) {
            std::cout << "Removing jump hook: " << jumpHooks[i]->description << std::endl;
            jumpHooks[i]->restoreStolenBytes();
        }
            
        // Give hook functions time to exit before deallocating trampolines.
        Sleep(500);

        for (uint64_t i = 0; i < jumpHooks.size(); i++) {
            jumpHooks[i]->freeTrampoline();
            std::cout << "Deleting jump hook: " << jumpHooks[i]->description << std::endl;
            delete jumpHooks[i];
        }
    }

}
