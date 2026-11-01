#ifndef _LEGACY_STATE_HPP_
#define _LEGACY_STATE_HPP_

#include <stdint.h>

enum CommunicationBackendId : uint8_t {
    COMMS_BACKEND_UNSPECIFIED = 0,
    COMMS_BACKEND_GAMECUBE = 1,
};

typedef struct {
    union {
        uint32_t mappedButtons = 0;

        struct {
            bool a : 1;
            bool b : 1;
            bool x : 1;
            bool y : 1;
            bool buttonL : 1;
            bool buttonR : 1;
            bool triggerLDigital : 1;
            bool triggerRDigital : 1;
            bool start : 1;
            bool select : 1;
            bool home : 1;
            bool capture : 1;
            bool dpadUp : 1;
            bool dpadDown : 1;
            bool dpadLeft : 1;
            bool dpadRight : 1;
            bool leftStickClick : 1;
            bool rightStickClick : 1;
            bool leftStickLeft : 1;
            bool leftStickRight : 1;
            bool leftStickUp : 1;
            bool leftStickDown : 1;
            bool rightStickLeft : 1;
            bool rightStickRight : 1;
            bool rightStickUp : 1;
            bool rightStickDown : 1;
            bool modX : 1;
            bool modY : 1;
        };
    };

    union {
        uint8_t analog_axes[6] = {128, 128, 128, 128, 0, 0};

        struct {
            uint8_t leftStickX;
            uint8_t leftStickY;
            uint8_t rightStickX;
            uint8_t rightStickY;
            uint8_t triggerLAnalog;
            uint8_t triggerRAnalog;
        };
    };

    uint64_t counter = 0;
} InputState;

typedef struct {
    union {
        uint32_t buttons = 0;

        struct {
            bool a : 1;
            bool b : 1;
            bool x : 1;
            bool y : 1;
            bool buttonL : 1;
            bool buttonR : 1;
            bool triggerLDigital : 1;
            bool triggerRDigital : 1;
            bool start : 1;
            bool select : 1;
            bool home : 1;
            bool capture : 1;
            bool dpadUp : 1;
            bool dpadDown : 1;
            bool dpadLeft : 1;
            bool dpadRight : 1;
            bool leftStickClick : 1;
            bool rightStickClick : 1;
            bool leftStickLeft : 1;
            bool leftStickRight : 1;
            bool leftStickUp : 1;
            bool leftStickDown : 1;
            bool rightStickLeft : 1;
            bool rightStickRight : 1;
            bool rightStickUp : 1;
            bool rightStickDown : 1;
            bool modX : 1;
            bool modY : 1;
        };
    };

    union {
        uint8_t analog_axes[6] = {128, 128, 128, 128, 0, 0};

        struct {
            uint8_t leftStickX;
            uint8_t leftStickY;
            uint8_t rightStickX;
            uint8_t rightStickY;
            uint8_t triggerLAnalog;
            uint8_t triggerRAnalog;
        };
    };
} OutputState;

#endif
