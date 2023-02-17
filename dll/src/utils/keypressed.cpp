#include "../../pch.h"
#include "./headers/keypressed.h"

bool keypressed( char vk ) {
    static bool wasPressed[0xFF] = {};
    int isPressed = GetAsyncKeyState( vk ) != 0;
    int result = !wasPressed[vk] && isPressed;
    wasPressed[vk] = isPressed;
    return result;
}