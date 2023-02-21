#include "../../pch.h"

namespace Hook {

    struct JumpHook {
        const char* description;
        UINT_PTR address;
        size_t numStolenBytes;
        UINT_PTR trampolineAddress;

        char* stolenBytes;

        JumpHook(
            const char* description,
            UINT_PTR address,
            size_t numStolenBytes,
            UINT_PTR trampolineAddress,
            UINT_PTR& returnAddress
        );

        JumpHook(
            const char* description,
            UINT_PTR address,
            size_t numStolenBytes,
            UINT_PTR trampolineAddress
        );

        void saveStolenBytes();
        void restoreStolenBytes();
        void hook();
        void unhook();
    };

    UINT_PTR getJumpDestination( UINT_PTR instructionAddress );

    void cleanupHooks();

}