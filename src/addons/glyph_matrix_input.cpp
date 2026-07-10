#include "addons/glyph_matrix_input.h"
#include "addons/turbo.h"

#include "addons/display.h"
#include "gamepad.h"
#include "glyph/glyph_profiles.h"
#include "helper.h"
#include "storagemanager.h"
#include "types.h"

#include "pico/stdlib.h"

#include <cstring>
#include <math.h>

namespace
{
constexpr uint8_t kRows = 4;
constexpr uint8_t kCols = 11;
constexpr uint32_t kMenuRepeatMs = 150;

constexpr uint8_t kRowPins[kRows] = {26, 25, 24, 23};
constexpr uint8_t kColPins[kCols] = {15, 14, 13, 12, 16, 17, 21, 20, 19, 18, 22};

constexpr uint8_t kMenuRow = 0;
constexpr uint8_t kMenuBackCol = 0;   // MB1
constexpr uint8_t kMenuUpCol = 1;     // MB2
constexpr uint8_t kMenuDownCol = 2;   // MB3
constexpr uint8_t kMenuSelectCol = 3; // MB4
constexpr uint8_t kGlyphButtonLF1 = 1;
constexpr uint8_t kGlyphButtonLF2 = 2;
constexpr uint8_t kGlyphButtonLF3 = 3;
constexpr uint8_t kGlyphButtonLF4 = 4;
constexpr uint8_t kGlyphButtonLF6 = 6;
constexpr uint8_t kGlyphButtonLF7 = 7;
constexpr uint8_t kGlyphButtonLF8 = 8;
constexpr uint8_t kGlyphButtonRF1 = 17;
constexpr uint8_t kGlyphButtonRF2 = 18;
constexpr uint8_t kGlyphButtonRF3 = 19;
constexpr uint8_t kGlyphButtonRF4 = 20;
constexpr uint8_t kGlyphButtonRF5 = 21;
constexpr uint8_t kGlyphButtonRF6 = 22;
constexpr uint8_t kGlyphButtonRF7 = 23;
constexpr uint8_t kGlyphButtonRF8 = 24;
constexpr uint8_t kGlyphButtonRF9 = 25;
constexpr uint8_t kGlyphButtonLT1 = 33;
constexpr uint8_t kGlyphButtonLT2 = 34;
constexpr uint8_t kGlyphButtonLT5 = 37;
constexpr uint8_t kGlyphButtonLT6 = 38;
constexpr uint8_t kGlyphButtonRT1 = 41;
constexpr uint8_t kGlyphButtonRT2 = 42;
constexpr uint8_t kGlyphButtonRT3 = 43;
constexpr uint8_t kGlyphButtonRT4 = 44;
constexpr uint8_t kGlyphButtonRT5 = 45;
constexpr uint8_t kGlyphButtonCRight = 45;
constexpr uint8_t kGlyphButtonCUp = 44;
constexpr uint8_t kGlyphButtonCLeft = 43;
constexpr uint8_t kGlyphButtonCDown = 42;
constexpr uint8_t kGlyphButtonMB2 = 50;
constexpr uint8_t kGlyphButtonMB4 = 52;
constexpr uint8_t kGlyphButtonMB5 = 53;
constexpr uint8_t kGlyphButtonMB6 = 54;
constexpr uint8_t kGlyphButtonMB7 = 55;
constexpr uint8_t kFullAnalogMagnitude = 127;
constexpr uint8_t kNoCAngleSlot = 0xff;
constexpr uint8_t kTurboLedStateOff = 0;
constexpr uint8_t kTurboLedStateOn = 1;
bool glyphMatrixPinsInitialized = false;

uint8_t axis8(uint16_t value)
{
    return static_cast<uint8_t>(value >> 8);
}

uint8_t invertAxis8(uint16_t value)
{
    return static_cast<uint8_t>(0xFF - axis8(value));
}

bool glyphTurboFlicker = false;
uint32_t glyphTurboNextToggleMs = 0;
uint8_t glyphTurboLastShotCount = 0;
uint16_t glyphTurboButtonsMask = 0;
uint16_t glyphTurboLastPressedMask = 0;

enum class LegacyPlatformProfile : uint8_t {
    None,
    Melee,
    ProjectM,
    Ultimate,
    Rivals,
    Rivals2,
    Smash64,
};

struct GlyphResolvedOutputState {
    uint32_t buttons = 0;
    uint32_t aux = 0;
    uint8_t dpad = 0;
    uint16_t lx = GAMEPAD_JOYSTICK_MID;
    uint16_t ly = GAMEPAD_JOYSTICK_MID;
    uint16_t rx = GAMEPAD_JOYSTICK_MID;
    uint16_t ry = GAMEPAD_JOYSTICK_MID;
    uint8_t lt = 0;
    uint8_t rt = 0;
};

void applyExactModOutput(GlyphResolvedOutputState& state, uint8_t mask, bool rightStick, uint8_t xMagnitude, uint8_t yMagnitude);
void resolveGlyphOutputs(GlyphResolvedOutputState& state);
void ensureGlyphMatrixPinsInitialized();

int8_t maskHorizontalDirection(uint8_t mask)
{
    if (mask & GAMEPAD_MASK_LEFT) return -1;
    if (mask & GAMEPAD_MASK_RIGHT) return 1;
    return 0;
}

int8_t maskVerticalDirection(uint8_t mask)
{
    if (mask & GAMEPAD_MASK_UP) return -1;
    if (mask & GAMEPAD_MASK_DOWN) return 1;
    return 0;
}

void setProfileLabel(char* dest, size_t len, const char* label)
{
    strncpy(dest, label, len - 1);
    dest[len - 1] = '\0';
}

void resolvedOutputStateToGamepadState(
    const GlyphResolvedOutputState& output,
    GamepadState& state
) {
    state = {};
    state.buttons = output.buttons;
    state.aux = output.aux;
    state.dpad = output.dpad;
    state.lx = output.lx;
    state.ly = output.ly;
    state.rx = output.rx;
    state.ry = output.ry;
    state.lt = output.lt;
    state.rt = output.rt;
}

void resolvedOutputStateToLegacyInputState(
    const GlyphResolvedOutputState& output,
    bool hasAnalogTriggers,
    InputState& inputs
) {
    inputs = {};
    inputs.a = (output.buttons & GAMEPAD_MASK_B1) != 0;
    inputs.b = (output.buttons & GAMEPAD_MASK_B2) != 0;
    inputs.x = (output.buttons & GAMEPAD_MASK_B3) != 0;
    inputs.y = (output.buttons & GAMEPAD_MASK_B4) != 0;
    inputs.buttonL = (output.buttons & GAMEPAD_MASK_L1) != 0;
    inputs.buttonR = (output.buttons & GAMEPAD_MASK_R1) != 0;
    inputs.triggerLDigital = (output.buttons & GAMEPAD_MASK_L2) != 0;
    inputs.triggerRDigital = (output.buttons & GAMEPAD_MASK_R2) != 0;
    inputs.start = (output.buttons & GAMEPAD_MASK_S2) != 0;
    inputs.select = (output.buttons & GAMEPAD_MASK_S1) != 0;
    inputs.home = (output.buttons & GAMEPAD_MASK_A1) != 0;
    inputs.capture = (output.buttons & GAMEPAD_MASK_A2) != 0;
    inputs.dpadUp = (output.dpad & GAMEPAD_MASK_UP) != 0;
    inputs.dpadDown = (output.dpad & GAMEPAD_MASK_DOWN) != 0;
    inputs.dpadLeft = (output.dpad & GAMEPAD_MASK_LEFT) != 0;
    inputs.dpadRight = (output.dpad & GAMEPAD_MASK_RIGHT) != 0;
    inputs.leftStickClick = (output.buttons & GAMEPAD_MASK_L3) != 0;
    inputs.rightStickClick = (output.buttons & GAMEPAD_MASK_R3) != 0;
    inputs.leftStickLeft = output.lx < GAMEPAD_JOYSTICK_MID;
    inputs.leftStickRight = output.lx > GAMEPAD_JOYSTICK_MID;
    inputs.leftStickUp = output.ly < GAMEPAD_JOYSTICK_MID;
    inputs.leftStickDown = output.ly > GAMEPAD_JOYSTICK_MID;
    inputs.rightStickLeft = output.rx < GAMEPAD_JOYSTICK_MID;
    inputs.rightStickRight = output.rx > GAMEPAD_JOYSTICK_MID;
    inputs.rightStickUp = output.ry < GAMEPAD_JOYSTICK_MID;
    inputs.rightStickDown = output.ry > GAMEPAD_JOYSTICK_MID;
    inputs.leftStickX = axis8(output.lx);
    inputs.leftStickY = invertAxis8(output.ly);
    inputs.rightStickX = axis8(output.rx);
    inputs.rightStickY = invertAxis8(output.ry);
    inputs.triggerLAnalog = hasAnalogTriggers ? output.lt : (inputs.triggerLDigital ? 0xFF : 0x00);
    inputs.triggerRAnalog = hasAnalogTriggers ? output.rt : (inputs.triggerRDigital ? 0xFF : 0x00);
}

uint16_t analogValue(bool positive, uint8_t magnitude)
{
    if (positive) {
        return static_cast<uint16_t>(GAMEPAD_JOYSTICK_MID +
            (((GAMEPAD_JOYSTICK_MAX - GAMEPAD_JOYSTICK_MID) * magnitude) / kFullAnalogMagnitude));
    }

    return static_cast<uint16_t>(GAMEPAD_JOYSTICK_MID -
        (((GAMEPAD_JOYSTICK_MID - GAMEPAD_JOYSTICK_MIN) * magnitude) / kFullAnalogMagnitude));
}

uint16_t analogSignedValue(int16_t signedMagnitude)
{
    if (signedMagnitude >= 0) {
        return static_cast<uint16_t>(GAMEPAD_JOYSTICK_MID +
            (((GAMEPAD_JOYSTICK_MAX - GAMEPAD_JOYSTICK_MID) * signedMagnitude) / kFullAnalogMagnitude));
    }

    return static_cast<uint16_t>(GAMEPAD_JOYSTICK_MID -
        (((GAMEPAD_JOYSTICK_MID - GAMEPAD_JOYSTICK_MIN) * -signedMagnitude) / kFullAnalogMagnitude));
}

uint8_t activeCAngleSlot(const bool glyphPressed[61])
{
    if (glyphPressed[kGlyphButtonCRight]) return 0;
    if (glyphPressed[kGlyphButtonCUp]) return 1;
    if (glyphPressed[kGlyphButtonCLeft]) return 2;
    if (glyphPressed[kGlyphButtonCDown]) return 3;
    return kNoCAngleSlot;
}

LegacyPlatformProfile legacyPlatformProfile(uint32_t behaviorMode)
{
    switch (behaviorMode) {
        case GlyphProfiles::BehaviorMelee: return LegacyPlatformProfile::Melee;
        case GlyphProfiles::BehaviorProjectM: return LegacyPlatformProfile::ProjectM;
        case GlyphProfiles::BehaviorUltimate: return LegacyPlatformProfile::Ultimate;
        case GlyphProfiles::BehaviorRivals: return LegacyPlatformProfile::Rivals;
        case GlyphProfiles::BehaviorRivals2: return LegacyPlatformProfile::Rivals2;
        case GlyphProfiles::BehaviorSmash64: return LegacyPlatformProfile::Smash64;
        default: return LegacyPlatformProfile::None;
    }
}

GlyphProfiles::Action translateGlyphButtonsForInputMode(
    const GlyphProfiles::Action& action,
    InputMode inputMode
) {
    if (action.target != GlyphProfiles::Target::Button) {
        return action;
    }

    GlyphProfiles::Action translated = action;
    switch (inputMode) {
        case INPUT_MODE_SWITCH:
        case INPUT_MODE_SWITCH_PRO:
            switch (action.mask) {
                case GAMEPAD_MASK_B1: translated.mask = GAMEPAD_MASK_B2; break;
                case GAMEPAD_MASK_B2: translated.mask = GAMEPAD_MASK_B1; break;
                case GAMEPAD_MASK_B3: translated.mask = GAMEPAD_MASK_B4; break;
                case GAMEPAD_MASK_B4: translated.mask = GAMEPAD_MASK_B3; break;
                default: break;
            }
            break;
        case INPUT_MODE_GENERIC:
            switch (action.mask) {
                case GAMEPAD_MASK_B1: translated.mask = GAMEPAD_MASK_B1; break; // A -> HID 2
                case GAMEPAD_MASK_B2: translated.mask = GAMEPAD_MASK_B3; break; // B -> HID 1
                case GAMEPAD_MASK_B3: translated.mask = GAMEPAD_MASK_B4; break; // X -> HID 4
                case GAMEPAD_MASK_B4: translated.mask = GAMEPAD_MASK_B2; break; // Y -> HID 3
                case GAMEPAD_MASK_L1: translated.mask = GAMEPAD_MASK_L2; break; // L1 -> HID 7
                case GAMEPAD_MASK_R1: translated.mask = GAMEPAD_MASK_L1; break; // R1 -> HID 5
                case GAMEPAD_MASK_L2: translated.mask = GAMEPAD_MASK_R2; break; // L2 -> HID 8
                case GAMEPAD_MASK_R2: translated.mask = GAMEPAD_MASK_R1; break; // R2 -> HID 6
                case GAMEPAD_MASK_L3: translated.mask = GAMEPAD_MASK_R3; break; // L3 -> HID 12
                case GAMEPAD_MASK_R3: translated.mask = GAMEPAD_MASK_L3; break; // R3 -> HID 11
                default: break;
            }
            break;
        default:
            break;
    }

    return translated;
}

bool inputModeNeedsGlyphButtonTranslation(InputMode inputMode)
{
    switch (inputMode) {
        case INPUT_MODE_SWITCH:
        case INPUT_MODE_SWITCH_PRO:
        case INPUT_MODE_GENERIC:
            return true;
        default:
            return false;
    }
}

bool glyphHasTurboPinAssigned()
{
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        if (pinMappings[pin].action == GpioAction::BUTTON_PRESS_TURBO) {
            return true;
        }
    }
    return false;
}

bool glyphVirtualTurboAvailable(uint8_t profile)
{
    return GlyphProfiles::buttonAvailable(profile, kGlyphButtonMB2);
}

void resetGlyphVirtualTurboState()
{
    glyphTurboFlicker = false;
    glyphTurboNextToggleMs = 0;
    glyphTurboLastPressedMask = 0;
    glyphTurboButtonsMask = 0;
}

void applyGlyphVirtualTurbo(GlyphResolvedOutputState& state)
{
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
    const uint8_t profile = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;
    const bool hasTurboPinAssigned = glyphHasTurboPinAssigned();
    const bool virtualTurboAvailable = !hasTurboPinAssigned && glyphVirtualTurboAvailable(profile);

    if (hasTurboPinAssigned) {
        resetGlyphVirtualTurboState();
        return;
    }

    if (!turboOptions.enabled || !virtualTurboAvailable) {
        resetGlyphVirtualTurboState();
        glyphTurboLastShotCount = turboOptions.shotCount;
        if (gamepad != nullptr) {
            gamepad->turboState.buttons = 0;
        }
        Gamepad* processedGamepad = Storage::getInstance().GetProcessedGamepad();
        if (processedGamepad != nullptr) {
            processedGamepad->auxState.turbo.enabled = turboOptions.enabled;
            processedGamepad->auxState.turbo.active = 0;
            processedGamepad->auxState.turbo.activity = kTurboLedStateOff;
        }
        return;
    }

    const uint8_t shotCount = turboOptions.shotCount < 2 ? 2 : turboOptions.shotCount;
    const uint32_t intervalMs = 1000 / (static_cast<uint32_t>(shotCount) * 2);
    const uint32_t now = getMillis();

    if (glyphTurboLastShotCount != shotCount) {
        glyphTurboLastShotCount = shotCount;
        glyphTurboNextToggleMs = now + intervalMs;
        glyphTurboFlicker = false;
    } else if (glyphTurboNextToggleMs == 0) {
        glyphTurboNextToggleMs = now + intervalMs;
    } else if (now >= glyphTurboNextToggleMs) {
        glyphTurboFlicker = !glyphTurboFlicker;
        glyphTurboNextToggleMs = now + intervalMs;
    }

    const bool turboModifierHeld = GlyphMatrixInput::glyphPhysicalButtonPressed(kGlyphButtonMB2);
    const uint16_t buttonsPressed = state.buttons & TURBO_BUTTON_MASK;
    if (turboModifierHeld) {
        if (buttonsPressed && glyphTurboLastPressedMask != buttonsPressed) {
            glyphTurboButtonsMask ^= (glyphTurboLastPressedMask ^ buttonsPressed) & ~glyphTurboLastPressedMask;
            glyphTurboFlicker = false;
        }
        glyphTurboLastPressedMask = buttonsPressed;
    } else {
        glyphTurboLastPressedMask = 0;
    }

    if (glyphTurboFlicker) {
        state.buttons &= ~glyphTurboButtonsMask;
    }

    if (gamepad != nullptr) {
        gamepad->turboState.buttons = glyphTurboButtonsMask;
        Gamepad* processedGamepad = Storage::getInstance().GetProcessedGamepad();
        if (processedGamepad != nullptr) {
            processedGamepad->auxState.turbo.enabled = true;
            processedGamepad->auxState.turbo.active = 1;
            if (glyphTurboButtonsMask) {
                processedGamepad->auxState.turbo.activity =
                    (state.buttons & glyphTurboButtonsMask) ? (glyphTurboFlicker ? kTurboLedStateOff : kTurboLedStateOn)
                                                            : kTurboLedStateOn;
            } else {
                processedGamepad->auxState.turbo.activity = kTurboLedStateOff;
            }
        }
    }
}

bool maskHasHorizontal(uint8_t mask)
{
    return (mask & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) != 0;
}

bool maskHasVertical(uint8_t mask)
{
    return (mask & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) != 0;
}

void applyLegacyLeftStickModifier(
    GlyphResolvedOutputState& state,
    uint8_t mask,
    LegacyPlatformProfile profile,
    const GlyphProfiles::ModProfileState& modProfile,
    bool modX,
    bool modY
) {
    if ((!modX && !modY) || mask == 0) {
        return;
    }

    const bool horizontal = maskHasHorizontal(mask);
    const bool vertical = maskHasVertical(mask);
    const bool diagonal = horizontal && vertical;

    uint8_t xMagnitude = kFullAnalogMagnitude;
    uint8_t yMagnitude = kFullAnalogMagnitude;

    switch (profile) {
        case LegacyPlatformProfile::Melee:
        case LegacyPlatformProfile::ProjectM:
        case LegacyPlatformProfile::Ultimate:
        case LegacyPlatformProfile::Rivals:
        case LegacyPlatformProfile::Rivals2:
            if (modX) {
                if (horizontal) xMagnitude = modProfile.modXHorizontal;
                if (vertical) yMagnitude = modProfile.modXVertical;
                if (diagonal) {
                    xMagnitude = modProfile.modXDiagonalX;
                    yMagnitude = modProfile.modXDiagonalY;
                }
            } else if (modY) {
                if (horizontal) xMagnitude = modProfile.modYHorizontal;
                if (vertical) yMagnitude = modProfile.modYVertical;
                if (diagonal) {
                    xMagnitude = modProfile.modYDiagonalX;
                    yMagnitude = modProfile.modYDiagonalY;
                }
            }
            break;

        case LegacyPlatformProfile::Smash64:
        case LegacyPlatformProfile::None:
        default:
            return;
    }

    applyExactModOutput(state, mask, false, xMagnitude, yMagnitude);
}

void applyExactModOutput(GlyphResolvedOutputState& state, uint8_t mask, bool rightStick, uint8_t xMagnitude, uint8_t yMagnitude)
{
    uint16_t& x = rightStick ? state.rx : state.lx;
    uint16_t& y = rightStick ? state.ry : state.ly;
    const bool horizontal = (mask & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) != 0;
    const bool vertical = (mask & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) != 0;

    if (!horizontal && !vertical) {
        return;
    }

    if (horizontal && vertical) {
        x = analogSignedValue((mask & GAMEPAD_MASK_LEFT) ? -static_cast<int16_t>(xMagnitude) : static_cast<int16_t>(xMagnitude));
        y = analogSignedValue((mask & GAMEPAD_MASK_UP) ? -static_cast<int16_t>(yMagnitude) : static_cast<int16_t>(yMagnitude));
        return;
    }

    if (mask & GAMEPAD_MASK_LEFT) x = analogValue(false, xMagnitude);
    if (mask & GAMEPAD_MASK_RIGHT) x = analogValue(true, xMagnitude);
    if (mask & GAMEPAD_MASK_UP) y = analogValue(false, yMagnitude);
    if (mask & GAMEPAD_MASK_DOWN) y = analogValue(true, yMagnitude);
}

void applyAnalogOutput(GlyphResolvedOutputState& state, uint8_t mask, bool rightStick, const GlyphProfiles::ModProfileState& modProfile, uint8_t cAngleSlot, bool modX = false, bool modY = false)
{
    if (modX) {
        if (cAngleSlot < 4) {
            applyExactModOutput(state, mask, rightStick, modProfile.modXX[cAngleSlot], modProfile.modXY[cAngleSlot]);
        } else {
            applyExactModOutput(state, mask, rightStick, kFullAnalogMagnitude, kFullAnalogMagnitude);
        }
        return;
    }
    if (modY) {
        if (cAngleSlot < 4) {
            applyExactModOutput(state, mask, rightStick, modProfile.modYX[cAngleSlot], modProfile.modYY[cAngleSlot]);
        } else {
            applyExactModOutput(state, mask, rightStick, kFullAnalogMagnitude, kFullAnalogMagnitude);
        }
        return;
    }

    uint16_t& x = rightStick ? state.rx : state.lx;
    uint16_t& y = rightStick ? state.ry : state.ly;
    const uint8_t xMagnitude = kFullAnalogMagnitude;
    const uint8_t yMagnitude = kFullAnalogMagnitude;
    if (mask & GAMEPAD_MASK_LEFT) x = analogValue(false, xMagnitude);
    if (mask & GAMEPAD_MASK_RIGHT) x = analogValue(true, xMagnitude);
    if (mask & GAMEPAD_MASK_UP) y = analogValue(false, yMagnitude);
    if (mask & GAMEPAD_MASK_DOWN) y = analogValue(true, yMagnitude);
}

void applyDirectionalOutput(GlyphResolvedOutputState& state, uint8_t dpadMask, DpadMode dpadMode, const GlyphProfiles::ModProfileState& modProfile, uint8_t cAngleSlot, bool modX, bool modY)
{
    // GP2040 stick mode only redirects the main dpad target. Dedicated LS/RS
    // targets are handled separately and always write that stick directly.
    switch (dpadMode) {
        case DpadMode::DPAD_MODE_LEFT_ANALOG:
            applyAnalogOutput(state, dpadMask, false, modProfile, cAngleSlot, modX, modY);
            break;
        case DpadMode::DPAD_MODE_RIGHT_ANALOG:
            applyAnalogOutput(state, dpadMask, true, modProfile, cAngleSlot, modX, modY);
            break;
        case DpadMode::DPAD_MODE_DIGITAL:
        default:
            state.dpad |= dpadMask;
            break;
    }
}

bool applyLegacyPlatformButton(
    GlyphResolvedOutputState& state,
    LegacyPlatformProfile profile,
    const GlyphProfiles::ModProfileState& modProfile,
    InputMode inputMode,
    uint8_t buttonId,
    bool modXPressed,
    bool dpadLayer,
    uint8_t& dpadOutput,
    uint8_t& leftAnalogOutput,
    uint8_t& rightAnalogOutput
) {
    auto setButton = [&](uint32_t mask) {
        const GlyphProfiles::Action translated = translateGlyphButtonsForInputMode(
            {GlyphProfiles::Target::Button, mask},
            inputMode
        );
        state.buttons |= translated.mask;
    };

    auto setDigitalTriggerL = [&]() {
        setButton(GAMEPAD_MASK_L2);
        if (state.lt < 140) {
            state.lt = 140;
        }
        return true;
    };

    auto setDigitalTriggerR = [&]() {
        setButton(GAMEPAD_MASK_R2);
        if (state.rt < 140) {
            state.rt = 140;
        }
        return true;
    };

    switch (buttonId) {
        case kGlyphButtonMB7:
            setButton(GAMEPAD_MASK_S2);
            return true;
        case kGlyphButtonMB6:
            setButton(GAMEPAD_MASK_S1);
            return true;
        case kGlyphButtonMB5:
            setButton(GAMEPAD_MASK_A1);
            return true;
        case kGlyphButtonMB4:
            setButton(GAMEPAD_MASK_A2);
            return true;
        default:
            break;
    }

    switch (profile) {
        case LegacyPlatformProfile::Melee:
        case LegacyPlatformProfile::ProjectM:
        case LegacyPlatformProfile::Ultimate:
        case LegacyPlatformProfile::Rivals:
        case LegacyPlatformProfile::Rivals2:
            switch (buttonId) {
                case kGlyphButtonLF3: leftAnalogOutput |= GAMEPAD_MASK_LEFT; return true;
                case kGlyphButtonLF1: leftAnalogOutput |= GAMEPAD_MASK_RIGHT; return true;
                case kGlyphButtonLF2: leftAnalogOutput |= GAMEPAD_MASK_DOWN; return true;
                case kGlyphButtonRF4: leftAnalogOutput |= GAMEPAD_MASK_UP; return true;

                case kGlyphButtonRT3:
                    if (dpadLayer) dpadOutput |= GAMEPAD_MASK_LEFT;
                    else rightAnalogOutput |= GAMEPAD_MASK_LEFT;
                    return true;
                case kGlyphButtonRT5:
                    if (dpadLayer) dpadOutput |= GAMEPAD_MASK_RIGHT;
                    else rightAnalogOutput |= GAMEPAD_MASK_RIGHT;
                    return true;
                case kGlyphButtonRT2:
                    if (dpadLayer) dpadOutput |= GAMEPAD_MASK_DOWN;
                    else rightAnalogOutput |= GAMEPAD_MASK_DOWN;
                    return true;
                case kGlyphButtonRT4:
                    if (dpadLayer) dpadOutput |= GAMEPAD_MASK_UP;
                    else rightAnalogOutput |= GAMEPAD_MASK_UP;
                    return true;

                case kGlyphButtonLF8: dpadOutput |= GAMEPAD_MASK_LEFT; return true;
                case kGlyphButtonLF6: dpadOutput |= GAMEPAD_MASK_RIGHT; return true;

                case kGlyphButtonRF7:
                    if (profile == LegacyPlatformProfile::Melee) {
                        if (state.rt < modProfile.lightShield1) {
                            state.rt = modProfile.lightShield1;
                        }
                    } else if (profile == LegacyPlatformProfile::Rivals || profile == LegacyPlatformProfile::Rivals2) {
                        setButton(GAMEPAD_MASK_L3);
                    } else {
                        dpadOutput |= GAMEPAD_MASK_DOWN;
                    }
                    return true;
                case kGlyphButtonRF8:
                    if (profile == LegacyPlatformProfile::Melee) {
                        if (state.rt < modProfile.lightShield2) {
                            state.rt = modProfile.lightShield2;
                        }
                    } else if (profile == LegacyPlatformProfile::Rivals || profile == LegacyPlatformProfile::Rivals2) {
                        setButton(GAMEPAD_MASK_R3);
                    } else {
                        dpadOutput |= GAMEPAD_MASK_UP;
                    }
                    return true;
                case kGlyphButtonRF9:
                    if (profile == LegacyPlatformProfile::ProjectM) {
                        if (state.rt < modProfile.lightShield1) {
                            state.rt = modProfile.lightShield1;
                        }
                        return true;
                    }
                    if (profile == LegacyPlatformProfile::Rivals ||
                        profile == LegacyPlatformProfile::Rivals2) {
                        setButton(GAMEPAD_MASK_L1);
                        return true;
                    }
                    return false;

                case kGlyphButtonLF7:
                    if (profile == LegacyPlatformProfile::Melee) {
                        dpadOutput |= GAMEPAD_MASK_DOWN;
                        return true;
                    }
                    if (profile == LegacyPlatformProfile::Rivals || profile == LegacyPlatformProfile::Rivals2) {
                        dpadOutput |= GAMEPAD_MASK_DOWN;
                        return true;
                    }
                    return false;
                case kGlyphButtonLT6:
                    if (profile == LegacyPlatformProfile::Melee) {
                        dpadOutput |= GAMEPAD_MASK_UP;
                        return true;
                    }
                    if (profile == LegacyPlatformProfile::Rivals || profile == LegacyPlatformProfile::Rivals2) {
                        dpadOutput |= GAMEPAD_MASK_UP;
                        return true;
                    }
                    return false;
                case kGlyphButtonLT5:
                    if (profile == LegacyPlatformProfile::Rivals2) {
                        // Local Rivals 2 uses the RoA button placement, so LT5
                        // should not fall through to the generic platform matrix.
                        return true;
                    }
                    return false;

                case kGlyphButtonRT1: setButton(GAMEPAD_MASK_B1); return true;
                case kGlyphButtonRF1: setButton(GAMEPAD_MASK_B2); return true;
                case kGlyphButtonRF2: setButton(GAMEPAD_MASK_B3); return true;
                case kGlyphButtonRF6: setButton(GAMEPAD_MASK_B4); return true;
                case kGlyphButtonRF3:
                    if (profile == LegacyPlatformProfile::ProjectM) {
                        setButton(modXPressed ? GAMEPAD_MASK_R1 : GAMEPAD_MASK_B1);
                    } else {
                        setButton(GAMEPAD_MASK_R1);
                    }
                    return true;
                case kGlyphButtonLF4: return setDigitalTriggerL();
                case kGlyphButtonRF5: return setDigitalTriggerR();
                default:
                    return false;
            }

        case LegacyPlatformProfile::Smash64:
            switch (buttonId) {
                case kGlyphButtonLF3:
                    leftAnalogOutput |= GAMEPAD_MASK_LEFT;
                    dpadOutput |= GAMEPAD_MASK_LEFT;
                    return true;
                case kGlyphButtonLF1:
                    leftAnalogOutput |= GAMEPAD_MASK_RIGHT;
                    dpadOutput |= GAMEPAD_MASK_RIGHT;
                    return true;
                case kGlyphButtonLF2:
                    leftAnalogOutput |= GAMEPAD_MASK_DOWN;
                    dpadOutput |= GAMEPAD_MASK_DOWN;
                    return true;
                case kGlyphButtonRF4:
                    leftAnalogOutput |= GAMEPAD_MASK_UP;
                    return true;
                case kGlyphButtonLT6:
                    dpadOutput |= GAMEPAD_MASK_UP;
                    return true;

                case kGlyphButtonRF7: rightAnalogOutput |= GAMEPAD_MASK_LEFT; return true;
                case kGlyphButtonRF8: rightAnalogOutput |= GAMEPAD_MASK_RIGHT; return true;
                case kGlyphButtonRF2: rightAnalogOutput |= GAMEPAD_MASK_DOWN; return true;
                case kGlyphButtonRF6: rightAnalogOutput |= GAMEPAD_MASK_UP; return true;

                case kGlyphButtonRT1: state.buttons |= GAMEPAD_MASK_B1; return true;
                case kGlyphButtonRF1: state.buttons |= GAMEPAD_MASK_B2; return true;
                case kGlyphButtonRF3: state.buttons |= GAMEPAD_MASK_R1; return true;
                case kGlyphButtonLF4: return setDigitalTriggerL();
                case kGlyphButtonRF5: return setDigitalTriggerR();
                default:
                    return false;
            }

        case LegacyPlatformProfile::None:
        default:
            return false;
    }
}

void applyProjectMPlatformOverrides(
    GlyphResolvedOutputState& state,
    const GlyphProfiles::ModProfileState& modProfile,
    uint8_t leftAnalogOutput,
    uint8_t rightAnalogOutput,
    bool modXPressed,
    bool modYPressed,
    bool shieldButtonPressed,
    bool rf1Pressed,
    bool rf3Pressed,
    bool rf5Pressed,
    bool rf9Pressed,
    bool horizontalSocd,
    uint8_t cAngleSlot
) {
    const bool horizontal = maskHasHorizontal(leftAnalogOutput);
    const bool vertical = maskHasVertical(leftAnalogOutput);
    const bool diagonal = horizontal && vertical;
    const int8_t xDirection = maskHorizontalDirection(leftAnalogOutput);
    const int8_t yDirection = maskVerticalDirection(leftAnalogOutput);

    const bool cHorizontal = maskHasHorizontal(rightAnalogOutput);
    const bool cDown = (rightAnalogOutput & GAMEPAD_MASK_DOWN) != 0;
    const int8_t cXDirection = maskHorizontalDirection(rightAnalogOutput);

    if (!modXPressed && !modYPressed && diagonal && yDirection > 0) {
        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 83);
        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 93);
    }

    if (modXPressed) {
        if (cHorizontal) {
            state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 65);
            state.ry = (yDirection == 0) ? GAMEPAD_JOYSTICK_MID
                                         : analogSignedValue(static_cast<int16_t>(yDirection) * 23);
        }

        if (diagonal && cAngleSlot == kNoCAngleSlot) {
            if (rf1Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 85);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 31);
            }

            if (rf5Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 82);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 35);
            }
        }
    }

    if (modYPressed && diagonal && cAngleSlot == kNoCAngleSlot) {
        if (rf1Pressed) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 28);
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 85);
        }

        if (rf5Pressed) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 51);
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 82);
        }
    }

    // C-stick ASDI slideoff overrides other c-stick modifiers for c-left/right + c-down.
    if (cHorizontal && cDown) {
        state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 35);
        state.ry = analogSignedValue(98);
    }

    if (horizontalSocd && !vertical && !shieldButtonPressed && xDirection != 0) {
        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 100);
    }

    if ((rf9Pressed || (rf3Pressed && !modXPressed)) && state.rt < modProfile.lightShield1) {
        state.rt = modProfile.lightShield1;
    }
}

void applyMeleePlatformOverrides(
    GlyphResolvedOutputState& state,
    const GlyphProfiles::ModProfileState& modProfile,
    uint8_t leftAnalogOutput,
    uint8_t rightAnalogOutput,
    bool modXPressed,
    bool modYPressed,
    bool shieldButtonPressed,
    bool rf1Pressed,
    bool horizontalSocd,
    uint8_t cAngleSlot
) {
    const bool horizontal = maskHasHorizontal(leftAnalogOutput);
    const bool vertical = maskHasVertical(leftAnalogOutput);
    const bool diagonal = horizontal && vertical;
    const int8_t xDirection = maskHorizontalDirection(leftAnalogOutput);
    const int8_t yDirection = maskVerticalDirection(leftAnalogOutput);

    const bool cHorizontal = maskHasHorizontal(rightAnalogOutput);
    const bool cVertical = maskHasVertical(rightAnalogOutput);
    const int8_t cXDirection = maskHorizontalDirection(rightAnalogOutput);
    const int8_t cYDirection = maskVerticalDirection(rightAnalogOutput);

    if (!modXPressed && !modYPressed && diagonal) {
        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 56);
        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 56);
        if (yDirection < 0 && shieldButtonPressed) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 56);
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 55);
        }
    }

    if (modXPressed) {
        if (horizontal) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXHorizontal);
        }
        if (vertical) {
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXVertical);
        }

        if (diagonal) {
            if (shieldButtonPressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 51);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 30);
            } else {
                if (rf1Pressed) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 73);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 31);
                    if (cAngleSlot == 3) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 70);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 36);
                    } else if (cAngleSlot == 2) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 68);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 42);
                    } else if (cAngleSlot == 1) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 59);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 43);
                    } else if (cAngleSlot == 0) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 51);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 43);
                    }
                } else {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXDiagonalX);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXDiagonalY);
                    if (cAngleSlot < 4) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXX[cAngleSlot]);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXY[cAngleSlot]);
                    }
                }
            }
        }

        if (cHorizontal) {
            state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 68);
            state.ry = (yDirection == 0) ? GAMEPAD_JOYSTICK_MID
                                         : analogSignedValue(static_cast<int16_t>(yDirection) * 42);
        }
    }

    if (modYPressed) {
        if (horizontal) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modYHorizontal);
        }
        if (vertical) {
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modYVertical);
        }
        if (rf1Pressed && xDirection != 0) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 80);
        }

        if (diagonal) {
            if (shieldButtonPressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 38);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 70);
                if (yDirection < 0) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 40);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 68);
                }
            } else {
                if (rf1Pressed) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 31);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 73);
                    if (cAngleSlot == 3) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 36);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 70);
                    } else if (cAngleSlot == 2) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 42);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 68);
                    } else if (cAngleSlot == 1) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 47);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 64);
                    } else if (cAngleSlot == 0) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 47);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 57);
                    }
                } else {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modYDiagonalX);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modYDiagonalY);
                    if (cAngleSlot < 4) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modYX[cAngleSlot]);
                        state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modYY[cAngleSlot]);
                    }
                }
            }
        }
    }

    if (cHorizontal && cVertical) {
        state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 42);
        state.ry = analogSignedValue(static_cast<int16_t>(cYDirection) * 68);
    }

    if (horizontalSocd && !vertical && xDirection != 0) {
        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 80);
    }
}

void applyUltimatePlatformOverrides(
    GlyphResolvedOutputState& state,
    const GlyphProfiles::ModProfileState& modProfile,
    uint8_t leftAnalogOutput,
    uint8_t rightAnalogOutput,
    bool modXPressed,
    bool modYPressed,
    bool shieldButtonPressed,
    bool rf1Pressed,
    bool rt1Pressed,
    uint8_t cAngleSlot
) {
    const bool horizontal = maskHasHorizontal(leftAnalogOutput);
    const bool vertical = maskHasVertical(leftAnalogOutput);
    const bool diagonal = horizontal && vertical;
    const int8_t xDirection = maskHorizontalDirection(leftAnalogOutput);
    const int8_t yDirection = maskVerticalDirection(leftAnalogOutput);

    const bool cHorizontal = maskHasHorizontal(rightAnalogOutput);
    const bool cVertical = maskHasVertical(rightAnalogOutput);
    const int8_t cXDirection = maskHorizontalDirection(rightAnalogOutput);
    const int8_t cYDirection = maskVerticalDirection(rightAnalogOutput);

    if (modXPressed) {
        if (horizontal) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXHorizontal);
            if (shieldButtonPressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 51);
            }
            if (rt1Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 36);
            }
        }
        if (vertical) {
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXVertical);
            if (shieldButtonPressed) {
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 51);
            }
        }
        if (diagonal && shieldButtonPressed) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 51);
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 30);
        }
        if (diagonal && !shieldButtonPressed) {
            if (rf1Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 67);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 44);
                if (cAngleSlot == 3) {
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 55);
                } else if (cAngleSlot == 2) {
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 49);
                } else if (cAngleSlot == 1) {
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 39);
                } else if (cAngleSlot == 0) {
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 35);
                }
            } else {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXDiagonalX);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXDiagonalY);
                if (cAngleSlot < 4) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modXX[cAngleSlot]);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modXY[cAngleSlot]);
                }
            }

            if (cHorizontal) {
                state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 127);
                state.ry = analogSignedValue(static_cast<int16_t>(yDirection) * 59);
            }
            if (rt1Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 36);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 26);
            }
        }
    }

    if (modYPressed) {
        if (horizontal) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 41);
            if (rt1Pressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 36);
            }
        }
        if (vertical) {
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 53);
            if (rt1Pressed) {
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 36);
            }
        }
        if (diagonal) {
            state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 35);
            state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 53);
            if (shieldButtonPressed) {
                state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 38);
                state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 70);
                if (xDirection < 0) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 40);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 68);
                }
            } else {
                if (rf1Pressed) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 44);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 67);
                    if (cAngleSlot == 3) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 55);
                    } else if (cAngleSlot == 2) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 49);
                    } else if (cAngleSlot == 1) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 39);
                    } else if (cAngleSlot == 0) {
                        state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 35);
                    }
                } else if (cAngleSlot < 4) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * modProfile.modYX[cAngleSlot]);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * modProfile.modYY[cAngleSlot]);
                }

                if (rt1Pressed) {
                    state.lx = analogSignedValue(static_cast<int16_t>(xDirection) * 34);
                    state.ly = analogSignedValue(static_cast<int16_t>(yDirection) * 38);
                }
            }
        }
    }

    if (cHorizontal && cVertical) {
        state.rx = analogSignedValue(static_cast<int16_t>(cXDirection) * 42);
        state.ry = analogSignedValue(static_cast<int16_t>(cYDirection) * 68);
    }
}
}

static GlyphMatrixInput::DebounceState debounce[kRows][kCols] = {};
static bool pressed[kRows][kCols] = {};
static bool menuControlHeld[4] = {};
static uint32_t menuControlLastSent[4] = {};
static uint32_t glyphButtonPressedAt[61] = {};
static bool glyphButtonPressedState[61] = {};
static bool glyphPhysicalButtonPressedState[61] = {};
static bool glyphModXPressedState = false;
static bool glyphModYPressedState = false;
static SOCDHistory glyphDpadSocdHistory = {};
static SOCDHistory glyphLeftSocdHistory = {};
static SOCDHistory glyphRightSocdHistory = {};
static uint8_t glyphSocdProfile = 0;

bool GlyphMatrixInput::available()
{
    return GLYPH_MATRIX_INPUT_ENABLED == 1;
}

void GlyphMatrixInput::setup()
{
    ensureProfiles();
    ensureGlyphMatrixPinsInitialized();
}

void GlyphMatrixInput::ensureProfiles()
{
    Storage& storage = Storage::getInstance();
    Config& config = storage.getConfig();

    setProfileLabel(config.gpioMappings.profileLabel, sizeof(config.gpioMappings.profileLabel), GlyphProfiles::name(1));

    const uint8_t gpioProfileCount = sizeof(config.profileOptions.gpioMappingsSets) / sizeof(config.profileOptions.gpioMappingsSets[0]);
    const uint8_t glyphMappingCount = GlyphProfiles::count() > 0 ? GlyphProfiles::count() - 1 : 0;
    const uint8_t mappedProfileCount = glyphMappingCount < gpioProfileCount ? glyphMappingCount : gpioProfileCount;

    if (config.profileOptions.gpioMappingsSets_count < mappedProfileCount) {
        config.profileOptions.gpioMappingsSets_count = mappedProfileCount;
    }

    for (uint8_t profile = 0; profile < mappedProfileCount; profile++) {
        GpioMappings& mapping = config.profileOptions.gpioMappingsSets[profile];
        mapping.enabled = true;
        if (mapping.pins_count == 0) {
            mapping.pins_count = NUM_BANK0_GPIOS;
        }
        setProfileLabel(mapping.profileLabel, sizeof(mapping.profileLabel), GlyphProfiles::name(profile + 2));
    }
}

void GlyphMatrixInput::preprocess()
{
    scan();
    handleMenuControls();

    applyProfileOptions();
}

void GlyphMatrixInput::applyProfileOptions()
{
    Storage& storage = Storage::getInstance();
    GamepadOptions& options = storage.getGamepadOptions();

    if (options.socdMode != SOCD_MODE_BYPASS) {
        options.socdMode = SOCD_MODE_BYPASS;
    }
}

void GlyphMatrixInput::process()
{
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    if (gamepad != nullptr) {
        apply(gamepad->state);
    }
}

void GlyphMatrixInput::postprocess(bool sent)
{
    (void)sent;
}

void GlyphMatrixInput::reinit()
{
    setup();
}

std::string GlyphMatrixInput::name()
{
    return "GlyphMatrixInput";
}

bool GlyphMatrixInput::glyphButtonPressed(uint8_t buttonId)
{
    return buttonId < sizeof(glyphButtonPressedState) && glyphButtonPressedState[buttonId];
}

bool GlyphMatrixInput::glyphPhysicalButtonPressed(uint8_t buttonId)
{
    return buttonId < sizeof(glyphPhysicalButtonPressedState) && glyphPhysicalButtonPressedState[buttonId];
}

void GlyphMatrixInput::refreshPhysicalStateForConfigMode()
{
    ensureGlyphMatrixPinsInitialized();
    memset(glyphPhysicalButtonPressedState, 0, sizeof(glyphPhysicalButtonPressedState));

    for (uint8_t row = 0; row < kRows; row++) {
        gpio_set_dir(kRowPins[row], GPIO_OUT);
        gpio_put(kRowPins[row], 0);
        sleep_us(5);

        for (uint8_t col = 0; col < kCols; col++) {
            if (gpio_get(kColPins[col])) {
                continue;
            }

            const uint8_t buttonId = GlyphProfiles::matrixButton(row, col);
            if (buttonId > 0 && buttonId < sizeof(glyphPhysicalButtonPressedState)) {
                glyphPhysicalButtonPressedState[buttonId] = true;
            }
        }

        gpio_set_dir(kRowPins[row], GPIO_IN);
        gpio_pull_up(kRowPins[row]);
    }
}

void GlyphMatrixInput::pollConsoleInputs(InputState& inputs, GamepadState* processedState)
{
    GlyphMatrixInput matrixInput;
    matrixInput.scan();
    matrixInput.handleMenuControls();
    matrixInput.applyProfileOptions();

    GlyphResolvedOutputState output;
    resolveGlyphOutputs(output);
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    const bool hasAnalogTriggers = gamepad != nullptr ? gamepad->hasAnalogTriggers : false;
    resolvedOutputStateToLegacyInputState(output, hasAnalogTriggers, inputs);

    if (gamepad != nullptr) {
        resolvedOutputStateToGamepadState(output, gamepad->state);
    }

    if (processedState != nullptr) {
        resolvedOutputStateToGamepadState(output, *processedState);
    }
}

bool GlyphMatrixInput::glyphModXPressed()
{
    return glyphModXPressedState;
}

bool GlyphMatrixInput::glyphModYPressed()
{
    return glyphModYPressedState;
}

bool GlyphMatrixInput::turboAvailable()
{
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    const uint8_t profile = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;
    const bool hasTurboPinAssigned = glyphHasTurboPinAssigned();
    return hasTurboPinAssigned || glyphVirtualTurboAvailable(profile);
}

void GlyphMatrixInput::scan()
{
    clearPressed();
    const uint32_t now = getMillis();

    for (uint8_t row = 0; row < kRows; row++) {
        gpio_set_dir(kRowPins[row], GPIO_OUT);
        gpio_put(kRowPins[row], 0);
        sleep_us(5);

        for (uint8_t col = 0; col < kCols; col++) {
            setCell(row, col, !gpio_get(kColPins[col]), now);
        }

        gpio_set_dir(kRowPins[row], GPIO_IN);
        gpio_pull_up(kRowPins[row]);
    }
}

namespace {
void ensureGlyphMatrixPinsInitialized()
{
    if (glyphMatrixPinsInitialized) {
        return;
    }

    for (uint8_t row = 0; row < kRows; row++) {
        gpio_init(kRowPins[row]);
        gpio_set_dir(kRowPins[row], GPIO_IN);
        gpio_pull_up(kRowPins[row]);
    }

    for (uint8_t col = 0; col < kCols; col++) {
        gpio_init(kColPins[col]);
        gpio_set_dir(kColPins[col], GPIO_IN);
        gpio_pull_up(kColPins[col]);
    }

    glyphMatrixPinsInitialized = true;
}

void resolveGlyphOutputs(GlyphResolvedOutputState& state)
{
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    const uint8_t profile = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;
    const InputMode inputMode = gamepad != nullptr ? gamepad->getOptions().inputMode : INPUT_MODE_XINPUT;
    const bool translateGlyphButtons = inputModeNeedsGlyphButtonTranslation(inputMode);
    const GlyphProfiles::ModProfileState& modProfile = GlyphProfiles::getModProfile(GlyphProfiles::modProfile(profile));
    if (gamepad != nullptr) {
        gamepad->hasAnalogTriggers = modProfile.analogTriggersEnabled;
    }
    const GlyphProfiles::Layout layout = GlyphProfiles::layout(profile);
    const LegacyPlatformProfile legacyPlatform = layout == GlyphProfiles::Layout::Platform ?
        legacyPlatformProfile(GlyphProfiles::behaviorMode(profile)) : LegacyPlatformProfile::None;
    bool originalGlyphPressed[61] = {};
    bool remappedGlyphPressed[61] = {};
    bool glyphPressed[61] = {};
    bool remappedPhysical[61] = {};
    uint8_t dpadOutput = 0;
    uint8_t leftAnalogOutput = 0;
    uint8_t rightAnalogOutput = 0;
    uint8_t cAngleSlot = kNoCAngleSlot;

    if (glyphSocdProfile != profile) {
        glyphSocdProfile = profile;
        glyphDpadSocdHistory = {};
        glyphLeftSocdHistory = {};
        glyphRightSocdHistory = {};
    }

    for (uint8_t row = 0; row < kRows; row++) {
        for (uint8_t col = 0; col < kCols; col++) {
            if (!pressed[row][col]) {
                continue;
            }

            const uint8_t buttonId = GlyphProfiles::matrixButton(row, col);
            if (buttonId > 0 && buttonId < sizeof(glyphPressed)) {
                originalGlyphPressed[buttonId] = true;
                glyphPressed[buttonId] = true;
            }
        }
    }

    for (uint8_t remapIndex = 0; remapIndex < GlyphProfiles::buttonRemapCount(profile); remapIndex++) {
        const GlyphProfiles::ButtonRemap& remap = GlyphProfiles::buttonRemap(profile, remapIndex);
        if (remap.physicalButton >= sizeof(glyphPressed) || remappedPhysical[remap.physicalButton]) {
            continue;
        }
        const bool shouldPress = originalGlyphPressed[remap.physicalButton] ||
                                 (remap.targetButton < sizeof(remappedGlyphPressed) && remappedGlyphPressed[remap.targetButton]);
        remappedPhysical[remap.physicalButton] = true;
        if (remap.targetButton > 0 && remap.targetButton < sizeof(remappedGlyphPressed)) {
            remappedGlyphPressed[remap.targetButton] = shouldPress;
            if (glyphButtonPressedAt[remap.targetButton] == 0) {
                glyphButtonPressedAt[remap.targetButton] = glyphButtonPressedAt[remap.physicalButton];
            }
        }
    }

    for (uint8_t buttonId = 1; buttonId < sizeof(glyphPressed); buttonId++) {
        glyphPressed[buttonId] = remappedGlyphPressed[buttonId] ||
                                 (originalGlyphPressed[buttonId] && !remappedPhysical[buttonId]);
    }

    bool modXPressed = layout == GlyphProfiles::Layout::Platform && glyphPressed[kGlyphButtonLT1];
    bool modYPressed = layout == GlyphProfiles::Layout::Platform && glyphPressed[kGlyphButtonLT2];

    memcpy(glyphButtonPressedState, glyphPressed, sizeof(glyphButtonPressedState));
    memcpy(glyphPhysicalButtonPressedState, originalGlyphPressed, sizeof(glyphPhysicalButtonPressedState));
    cAngleSlot = activeCAngleSlot(glyphPressed);

    for (uint8_t buttonId = 1; buttonId < sizeof(glyphPressed); buttonId++) {
        if (!glyphPressed[buttonId]) {
            continue;
        }

        // FW-Glyph treats LT1/LT2 as internal analog modifiers in platform
        // layouts; they are not emitted as controller buttons.
        if (layout == GlyphProfiles::Layout::Platform) {
            if (buttonId == kGlyphButtonLT1) {
                continue;
            }
            if (buttonId == kGlyphButtonLT2) {
                continue;
            }
        }

        if (legacyPlatform != LegacyPlatformProfile::None &&
            applyLegacyPlatformButton(
                state,
                legacyPlatform,
                modProfile,
                inputMode,
                buttonId,
                modXPressed,
                modXPressed && modYPressed,
                dpadOutput,
                leftAnalogOutput,
                rightAnalogOutput
            )) {
            continue;
        }

        const GlyphProfiles::Action rawAction = GlyphProfiles::buttonAction(profile, buttonId);
        const GlyphProfiles::Action action = translateGlyphButtons ?
            translateGlyphButtonsForInputMode(rawAction, inputMode) :
            rawAction;
        switch (action.target) {
            case GlyphProfiles::Target::Dpad:
                dpadOutput |= static_cast<uint8_t>(action.mask);
                break;
            case GlyphProfiles::Target::LeftAnalog:
                leftAnalogOutput |= static_cast<uint8_t>(action.mask);
                break;
            case GlyphProfiles::Target::RightAnalog:
                rightAnalogOutput |= static_cast<uint8_t>(action.mask);
                break;
            case GlyphProfiles::Target::Button:
                state.buttons |= action.mask;
                break;
            case GlyphProfiles::Target::Aux:
                state.aux |= action.mask;
                break;
            case GlyphProfiles::Target::None:
                break;
        }
    }

    glyphModXPressedState = modXPressed;
    glyphModYPressedState = modYPressed;

    const bool horizontalSocd = legacyPlatform == LegacyPlatformProfile::Melee
        ? (glyphPressed[kGlyphButtonLF3] && glyphPressed[kGlyphButtonLF1])
        : (leftAnalogOutput & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) == (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT);

    if (legacyPlatform != LegacyPlatformProfile::None &&
        legacyPlatform != LegacyPlatformProfile::ProjectM &&
        legacyPlatform != LegacyPlatformProfile::Melee &&
        legacyPlatform != LegacyPlatformProfile::Ultimate &&
        leftAnalogOutput != 0 && cAngleSlot == kNoCAngleSlot &&
        (modXPressed || modYPressed)) {
        applyLegacyLeftStickModifier(state, leftAnalogOutput, legacyPlatform, modProfile, modXPressed, modYPressed);
        leftAnalogOutput = 0;
    }

    dpadOutput = runSOCDCleaner(
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotDpadVertical),
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotDpadHorizontal),
        dpadOutput,
        glyphDpadSocdHistory
    );
    leftAnalogOutput = runSOCDCleaner(
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotLeftAnalogVertical),
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotLeftAnalogHorizontal),
        leftAnalogOutput,
        glyphLeftSocdHistory
    );
    rightAnalogOutput = runSOCDCleaner(
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotRightAnalogVertical),
        GlyphProfiles::logicalSocdMode(profile, GlyphProfiles::SocdLogicalSlotRightAnalogHorizontal),
        rightAnalogOutput,
        glyphRightSocdHistory
    );

    if (gamepad != nullptr) {
        applyDirectionalOutput(state, dpadOutput, gamepad->getOptions().dpadMode, modProfile, cAngleSlot, modXPressed, modYPressed);
    }
    applyAnalogOutput(state, leftAnalogOutput, false, modProfile, cAngleSlot, modXPressed, modYPressed);
    applyAnalogOutput(state, rightAnalogOutput, true, modProfile, cAngleSlot, modXPressed, modYPressed);
    if (legacyPlatform == LegacyPlatformProfile::ProjectM) {
        const bool shieldButtonPressed = glyphPressed[kGlyphButtonLF4] || glyphPressed[kGlyphButtonRF5];
        applyProjectMPlatformOverrides(
            state,
            modProfile,
            leftAnalogOutput,
            rightAnalogOutput,
            modXPressed,
            modYPressed,
            shieldButtonPressed,
            glyphPressed[kGlyphButtonRF1],
            glyphPressed[kGlyphButtonRF3],
            glyphPressed[kGlyphButtonRF5],
            glyphPressed[kGlyphButtonRF9],
            horizontalSocd,
            cAngleSlot
        );
    } else if (legacyPlatform == LegacyPlatformProfile::Melee) {
        const bool shieldButtonPressed = glyphPressed[kGlyphButtonLF4] || glyphPressed[kGlyphButtonRF5] ||
                                         glyphPressed[kGlyphButtonRF7] || glyphPressed[kGlyphButtonRF8];
        applyMeleePlatformOverrides(
            state,
            modProfile,
            leftAnalogOutput,
            rightAnalogOutput,
            modXPressed,
            modYPressed,
            shieldButtonPressed,
            glyphPressed[kGlyphButtonRF1],
            horizontalSocd,
            cAngleSlot
        );
    } else if (legacyPlatform == LegacyPlatformProfile::Ultimate) {
        const bool shieldButtonPressed = glyphPressed[kGlyphButtonLF4] || glyphPressed[kGlyphButtonRF5];
        applyUltimatePlatformOverrides(
            state,
            modProfile,
            leftAnalogOutput,
            rightAnalogOutput,
            modXPressed,
            modYPressed,
            shieldButtonPressed,
            glyphPressed[kGlyphButtonRF1],
            glyphPressed[kGlyphButtonRT1],
            cAngleSlot
        );
    }
    applyGlyphVirtualTurbo(state);
}
}

void GlyphMatrixInput::apply(GamepadState& state)
{
    GlyphResolvedOutputState output;
    resolveGlyphOutputs(output);
    resolvedOutputStateToGamepadState(output, state);
}

void GlyphMatrixInput::handleMenuControls()
{
    const bool controls[4] = {
        pressed[kMenuRow][kMenuBackCol],
        pressed[kMenuRow][kMenuUpCol],
        pressed[kMenuRow][kMenuDownCol],
        pressed[kMenuRow][kMenuSelectCol],
    };
    const GpioAction actions[4] = {
        GpioAction::MENU_NAVIGATION_BACK,
        GpioAction::MENU_NAVIGATION_UP,
        GpioAction::MENU_NAVIGATION_DOWN,
        GpioAction::MENU_NAVIGATION_SELECT,
    };

    const uint32_t now = getMillis();
    for (uint8_t i = 0; i < 4; i++) {
        if (!controls[i]) {
            menuControlHeld[i] = false;
            continue;
        }

        const bool canRepeat = actions[i] == GpioAction::MENU_NAVIGATION_UP ||
                               actions[i] == GpioAction::MENU_NAVIGATION_DOWN;
        if (!menuControlHeld[i] || (canRepeat && (now - menuControlLastSent[i]) >= kMenuRepeatMs)) {
            DisplayAddon::queueMenuAction(actions[i]);
            menuControlHeld[i] = true;
            menuControlLastSent[i] = now;
        }
    }
}

void GlyphMatrixInput::setCell(uint8_t row, uint8_t col, bool isPressed, uint32_t now)
{
    DebounceState& cell = debounce[row][col];
    if (cell.observed != isPressed) {
        cell.observed = isPressed;
        cell.changedAt = now;
    }

    if (cell.stable != cell.observed && (now - cell.changedAt) >= GLYPH_MATRIX_DEBOUNCE_MS) {
        cell.stable = cell.observed;
        const uint8_t buttonId = GlyphProfiles::matrixButton(row, col);
        if (buttonId > 0 && buttonId < sizeof(glyphButtonPressedAt)) {
            glyphButtonPressedAt[buttonId] = cell.stable ? now : 0;
        }
    }

    pressed[row][col] = cell.stable;
}

void GlyphMatrixInput::clearPressed()
{
    for (uint8_t row = 0; row < kRows; row++) {
        for (uint8_t col = 0; col < kCols; col++) {
            pressed[row][col] = false;
        }
    }
}
