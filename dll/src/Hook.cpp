#include "./headers/Hook.h"
#include "./utils/headers/AllocationUtils.h"
#include "./utils/headers/MathUtils.h"
#include "./utils/headers/common.h"

#define CALL   '\xE8'
#define JMP    '\xE9'
#define NOP    '\x90'

bool isValid32BitOffset( UINT_PTR offset ) {
    return offset >= MININT32 && offset <= MAXINT32;
}

inline void writeBytes( char** pDest, char* src, size_t count ) {
    memcpy( *pDest, src, count );
    *pDest += count;
}

template<typename T>
inline void write( char** pDest, T content ) {
    writeBytes( pDest, (char*) &content, sizeof( T ) );
}

inline void writeOffset( char** pDest, UINT_PTR address ) {
    INT_PTR offset = MathUtils::signedDifference( address - sizeof( DWORD ), (INT_PTR) *pDest );
    if ( isValid32BitOffset( offset ) )
        throw std::runtime_error( "Offset does not fit in 32-bit integer." );
    write( pDest, (int32_t) offset );
}

namespace Hook {

    std::vector<JumpHook*> jumpHooks;

    void JumpHook::saveStolenBytes() {
        if ( stolenBytes )
            return;
        stolenBytes = (char*) malloc( numStolenBytes );
        memcpy( stolenBytes, (char*) address, numStolenBytes );
    }

    void JumpHook::restoreStolenBytes() {
        if ( !AllocationUtils::isAllocated( address ) ) {
            std::cout << "Cannot restore original code. Hook site is no longer allocated.\n";
            return;
        }
        if ( stolenBytes )
            memcpyExecutable( (char*) address, stolenBytes, numStolenBytes );
    }

    void JumpHook::hook() {
        std::cout << "Adding jump hook: " << description << std::endl;
        std::cout << "Trampoline located at: " << std::hex << (UINT_PTR) trampolineAddress << std::endl << std::endl;

        saveStolenBytes();

        DWORD oldProtect;
        VirtualProtect( (void*) address, numStolenBytes, PAGE_EXECUTE_READWRITE, &oldProtect );

        // Make sure anything not overwritten by jump is a nop
        memset( (void*) address, NOP, numStolenBytes );
        char* head = (char*) address;

        // Jump to trampoline
        write( &head, JMP );
        writeOffset( &head, (UINT_PTR) trampolineAddress );

        VirtualProtect( (void*) address, numStolenBytes, oldProtect, &oldProtect );
    }

    void JumpHook::unhook() {
        std::cout << "Removing jump hook: " << description << std::endl;
        restoreStolenBytes();
        safeFree( stolenBytes );
    }

    JumpHook::JumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR trampolineAddress
    ):
        description( description ),
        address( address ),
        numStolenBytes( numStolenBytes ),
        trampolineAddress( trampolineAddress ),
        stolenBytes( nullptr ) {
        jumpHooks.emplace_back( this );
    }

    JumpHook::JumpHook(
        const char* description,
        UINT_PTR address,
        size_t numStolenBytes,
        UINT_PTR trampolineAddress,
        UINT_PTR& returnAddress
    ): JumpHook( description, address, numStolenBytes, trampolineAddress ) {
        returnAddress = address + numStolenBytes;
    }

    UINT_PTR getJumpDestination( UINT_PTR instructionAddress ) {
        char op = *(char*) instructionAddress;
        if ( op != JMP ) {
            std::cout << "Expected jump instruction at " << instructionAddress << "\n";
            return 0;
        }
        auto offsetAddress = instructionAddress + 1;
        return offsetAddress + sizeof( int32_t ) + *(int32_t*) ( offsetAddress );
    }

    void cleanupHooks() {
        for ( JumpHook* hook : jumpHooks )
            hook->unhook();
    }

}
