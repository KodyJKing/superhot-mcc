#include "HaloMCC.hpp"
#include "memory/Memory.hpp"

namespace HaloMCC {

    uintptr_t mccBase() {
        return (uintptr_t) GetModuleHandleA( NULL );
    }

    // True even when menu is not open. For example, when loading a map or when in the main menu.
    bool isGamePaused() {
        auto result = Memory::safeRead<bool>( (uintptr_t) (mccBase() + 0x3FFCA93U) );
        return result.has_value() && result.value();
    }

    // True only when in a map and the pause menu is open.
    bool isPauseMenuOpen() {
        auto result = Memory::safeRead<bool>( (uintptr_t) (mccBase() + 0x3FFCA9FU) );
        return result.has_value() && result.value();
    }

}