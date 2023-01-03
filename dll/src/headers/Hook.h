#include "../pch.h"

// === Opcodes =======
#define CALL   '\xE8'
#define JMP    '\xE9'
#define NOP    '\x90'
#define PUSHF  '\x9C'
#define POPF   '\x9D'
#define PUSHA  '\x60'
#define POPA   '\x61'
// ===================

// === Jump hook flags ==========
#define HK_JUMP         0b00000001
#define HK_PUSH_STATE   0b00000010
#define HK_STOLEN_AFTER 0b00000100
// ===============================
    
namespace Hook {

    struct JumpHook {
        const char* description;
        UINT_PTR address;
        size_t numStolenBytes; // Rather, the number of bytes overwritten. Not necessarily executed.

        char* trampolineBytes;
        UINT_PTR trampolineReturn;
        size_t trampolineSize;
        char* stolenBytes;
        char* transplantedBytes;
        int32_t transplantOffset;
        
        void allocTrampoline(size_t size);
        void protectTrampoline();
        void unprotectTrampoline();
        void freeTrampoline();
        void restoreStolenBytes();
        void hook();
        void unhook();
        void writeStolenBytes(char ** head);
        void fixStolenOffset(size_t stolenByteIndex);
        void writeReturnJump(char ** head);
        void writePushState(char ** head);
        void writePopState(char ** head);
        void writeJump(char ** head, UINT_PTR hookFunc);
        void writeCall(char ** head, UINT_PTR hookFunc);
        void writeAbsoluteJump(char** head, UINT_PTR hookFunc);
        void writeAbsoluteCall(char** head, UINT_PTR hookFunc);
    };

    JumpHook addJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags,
        UINT_PTR* returnAddress
    );

    JumpHook addJumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR hookFunc,
        DWORD flags
    );

    void removeAllJumpHookRecords();

}