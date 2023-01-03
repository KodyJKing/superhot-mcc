#include "../../pch.h"
#include "./headers/keypressed.h"

bool wasPressed[0xFF] = {};

bool keypressed(char vk) {
    int isPressed = GetAsyncKeyState(vk) != 0;
    int result = !wasPressed[vk] && isPressed;
    wasPressed[vk] = isPressed;
    return result;
}