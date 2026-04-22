/*
 * Glyph MK6 board definition for GP2040-CE.
 *
 * The button matrix is scanned by the GlyphMatrixInput addon, so the matrix,
 * OLED, LED, and legacy console pins are reserved from GP2040's normal
 * one-GPIO-per-button mapper.
 */

#ifndef GLYPH_MK6_BOARD_CONFIG_H_
#define GLYPH_MK6_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Glyph MK6"

// Glyph hardware pins owned by addons/platform code.
#define GPIO_PIN_02 GpioAction::ASSIGNED_TO_ADDON // OLED SDA
#define GPIO_PIN_03 GpioAction::ASSIGNED_TO_ADDON // OLED SCL
#define GPIO_PIN_08 GpioAction::ASSIGNED_TO_ADDON // Joybus/NES data
#define GPIO_PIN_09 GpioAction::ASSIGNED_TO_ADDON // NES clock
#define GPIO_PIN_10 GpioAction::ASSIGNED_TO_ADDON // NES latch
#define GPIO_PIN_11 GpioAction::ASSIGNED_TO_ADDON // RGB LEDs

#define GPIO_PIN_12 GpioAction::ASSIGNED_TO_ADDON // Matrix col 3
#define GPIO_PIN_13 GpioAction::ASSIGNED_TO_ADDON // Matrix col 2
#define GPIO_PIN_14 GpioAction::ASSIGNED_TO_ADDON // Matrix col 1
#define GPIO_PIN_15 GpioAction::ASSIGNED_TO_ADDON // Matrix col 0
#define GPIO_PIN_16 GpioAction::ASSIGNED_TO_ADDON // Matrix col 4
#define GPIO_PIN_17 GpioAction::ASSIGNED_TO_ADDON // Matrix col 5
#define GPIO_PIN_18 GpioAction::ASSIGNED_TO_ADDON // Matrix col 9
#define GPIO_PIN_19 GpioAction::ASSIGNED_TO_ADDON // Matrix col 8
#define GPIO_PIN_20 GpioAction::ASSIGNED_TO_ADDON // Matrix col 7
#define GPIO_PIN_21 GpioAction::ASSIGNED_TO_ADDON // Matrix col 6
#define GPIO_PIN_22 GpioAction::ASSIGNED_TO_ADDON // Matrix col 10
#define GPIO_PIN_23 GpioAction::ASSIGNED_TO_ADDON // Matrix row 3
#define GPIO_PIN_24 GpioAction::ASSIGNED_TO_ADDON // Matrix row 2
#define GPIO_PIN_25 GpioAction::ASSIGNED_TO_ADDON // Matrix row 1
#define GPIO_PIN_26 GpioAction::ASSIGNED_TO_ADDON // Matrix row 0

// Keyboard fallback. The matrix addon is responsible for gamepad input.
#define KEY_DPAD_UP     HID_KEY_ARROW_UP
#define KEY_DPAD_DOWN   HID_KEY_ARROW_DOWN
#define KEY_DPAD_RIGHT  HID_KEY_ARROW_RIGHT
#define KEY_DPAD_LEFT   HID_KEY_ARROW_LEFT
#define KEY_BUTTON_B1   HID_KEY_Z
#define KEY_BUTTON_B2   HID_KEY_X
#define KEY_BUTTON_R2   HID_KEY_C
#define KEY_BUTTON_L2   HID_KEY_V
#define KEY_BUTTON_B3   HID_KEY_A
#define KEY_BUTTON_B4   HID_KEY_S
#define KEY_BUTTON_R1   HID_KEY_D
#define KEY_BUTTON_L1   HID_KEY_F
#define KEY_BUTTON_S1   HID_KEY_5
#define KEY_BUTTON_S2   HID_KEY_1
#define KEY_BUTTON_L3   HID_KEY_EQUAL
#define KEY_BUTTON_R3   HID_KEY_MINUS
#define KEY_BUTTON_A1   HID_KEY_9
#define KEY_BUTTON_A2   HID_KEY_F2
#define KEY_BUTTON_FN   -1

#define BOARD_LEDS_PIN 11
#define LED_BRIGHTNESS_MAXIMUM 128
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1
#define LED_COUNT 76
#define CASE_RGB_TYPE CASE_RGB_TYPE_AMBIENT
#define CASE_RGB_INDEX 0
#define CASE_RGB_COUNT LED_COUNT

#define HAS_I2C_DISPLAY 1
#define I2C1_ENABLED 1
#define I2C1_PIN_SDA 2
#define I2C1_PIN_SCL 3
#define DISPLAY_I2C_BLOCK i2c1
#define DISPLAY_I2C_ADDR 0x3C
#define DISPLAY_SIZE GPGFX_DisplaySize::SIZE_128x64
#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
#define GLYPH_DISPLAY_SCREEN 1

#define GLYPH_MATRIX_INPUT_ENABLED 1
#define GLYPH_MATRIX_DEBOUNCE_MS 5

#endif
