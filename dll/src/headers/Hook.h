#include "../../pch.h"

namespace Hook {

    class JumpHook {

        public:

        ~JumpHook();

        JumpHook(
            const char* description,
            UINT_PTR address,
            size_t numStolenBytes,
            UINT_PTR trampolineAddress
        );

        JumpHook(
            const char* description,
            UINT_PTR address,
            size_t numStolenBytes,
            UINT_PTR trampolineAddress,
            UINT_PTR& returnAddress
        );

        void hook();
        void unhook();

        private:

        const char* description;
        UINT_PTR address;
        size_t numStolenBytes;
        UINT_PTR trampolineAddress;

        std::vector<char> stolenBytes;

        void saveStolenBytes();
        void restoreStolenBytes();
    };

    UINT_PTR getJumpDestination( UINT_PTR instructionAddress );

    void cleanupHooks();

}