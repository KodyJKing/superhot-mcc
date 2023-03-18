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

        private:

        const char* description;
        UINT_PTR address;
        size_t numStolenBytes;
        UINT_PTR trampolineAddress;

        bool isHooked;

        std::vector<char> stolenBytes;

        void saveStolenBytes();
        void restoreStolenBytes();

        void hook();
        void unhook();
    };

    UINT_PTR getJumpDestination( UINT_PTR instructionAddress );

}

using HookPointer = std::unique_ptr<Hook::JumpHook>;