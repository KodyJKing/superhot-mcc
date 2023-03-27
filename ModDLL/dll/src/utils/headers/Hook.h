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

    class VirtualTableHook {

        public:

        ~VirtualTableHook();

        VirtualTableHook(
            const char* description,
            void** vtable,
            size_t methodIndex,
            void* hookFunction,
            void** originalFunction
        );

        private:

        const char* description;
        void** vtable;
        size_t methodIndex;
        void* hookFunction;
        void* _originalFunction;
    };

}

using HookPointer = std::unique_ptr<Hook::JumpHook>;
using VHookPointer = std::unique_ptr<Hook::VirtualTableHook>;