#include "drivers/legacy/GamepadInputSource.hpp"

#include "gamepad/GamepadState.h"

namespace {
static inline uint8_t axis8(uint16_t value) {
    return static_cast<uint8_t>(value >> 8);
}
}

void GamepadInputSource::UpdateInputs(InputState &inputs) {
    if (currentGamepad == nullptr) {
        return;
    }

    inputs.mappedButtons = 0;
    inputs.a = currentGamepad->pressedB1();
    inputs.b = currentGamepad->pressedB2();
    inputs.x = currentGamepad->pressedB3();
    inputs.y = currentGamepad->pressedB4();
    inputs.buttonL = currentGamepad->pressedL1();
    inputs.buttonR = currentGamepad->pressedR1();
    inputs.triggerLDigital = currentGamepad->pressedL2();
    inputs.triggerRDigital = currentGamepad->pressedR2();
    inputs.start = currentGamepad->pressedS2();
    inputs.select = currentGamepad->pressedS1();
    inputs.home = currentGamepad->pressedA1();
    inputs.capture = currentGamepad->pressedA2();
    inputs.dpadUp = currentGamepad->pressedUp();
    inputs.dpadDown = currentGamepad->pressedDown();
    inputs.dpadLeft = currentGamepad->pressedLeft();
    inputs.dpadRight = currentGamepad->pressedRight();
    inputs.leftStickClick = currentGamepad->pressedL3();
    inputs.rightStickClick = currentGamepad->pressedR3();
    inputs.leftStickLeft = currentGamepad->state.lx < GAMEPAD_JOYSTICK_MID;
    inputs.leftStickRight = currentGamepad->state.lx > GAMEPAD_JOYSTICK_MID;
    inputs.leftStickUp = currentGamepad->state.ly > GAMEPAD_JOYSTICK_MID;
    inputs.leftStickDown = currentGamepad->state.ly < GAMEPAD_JOYSTICK_MID;
    inputs.rightStickLeft = currentGamepad->state.rx < GAMEPAD_JOYSTICK_MID;
    inputs.rightStickRight = currentGamepad->state.rx > GAMEPAD_JOYSTICK_MID;
    inputs.rightStickUp = currentGamepad->state.ry > GAMEPAD_JOYSTICK_MID;
    inputs.rightStickDown = currentGamepad->state.ry < GAMEPAD_JOYSTICK_MID;

    inputs.leftStickX = axis8(currentGamepad->state.lx);
    inputs.leftStickY = axis8(currentGamepad->state.ly);
    inputs.rightStickX = axis8(currentGamepad->state.rx);
    inputs.rightStickY = axis8(currentGamepad->state.ry);
    inputs.triggerLAnalog = currentGamepad->hasAnalogTriggers
        ? currentGamepad->state.lt
        : (currentGamepad->pressedL2() ? 0xFF : 0x00);
    inputs.triggerRAnalog = currentGamepad->hasAnalogTriggers
        ? currentGamepad->state.rt
        : (currentGamepad->pressedR2() ? 0xFF : 0x00);
}
