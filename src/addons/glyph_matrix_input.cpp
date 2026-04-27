#include "addons/glyph_matrix_input.h"

#include "eventmanager.h"
#include "gamepad.h"
#include "glyph/glyph_profiles.h"
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
constexpr uint8_t kGlyphButtonMB4 = 52;
constexpr uint8_t kGlyphButtonMB5 = 53;
constexpr uint8_t kGlyphButtonMB6 = 54;
constexpr uint8_t kGlyphButtonMB7 = 55;
constexpr uint8_t kFullAnalogMagnitude = 127;
constexpr uint8_t kNoCAngleSlot = 0xff;

enum class LegacyPlatformProfile : uint8_t {
    None,
    Melee,
    ProjectM,
    Ultimate,
    Rivals,
    Rivals2,
    Smash64,
};

void applyExactModOutput(GamepadState& state, uint8_t mask, bool rightStick, uint8_t xMagnitude, uint8_t yMagnitude);

void setProfileLabel(char* dest, size_t len, const char* label)
{
    strncpy(dest, label, len - 1);
    dest[len - 1] = '\0';
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

bool maskHasHorizontal(uint8_t mask)
{
    return (mask & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT)) != 0;
}

bool maskHasVertical(uint8_t mask)
{
    return (mask & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN)) != 0;
}

void applyLegacyLeftStickModifier(
    GamepadState& state,
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

void applyExactModOutput(GamepadState& state, uint8_t mask, bool rightStick, uint8_t xMagnitude, uint8_t yMagnitude)
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

void applyAnalogOutput(GamepadState& state, uint8_t mask, bool rightStick, const GlyphProfiles::ModProfileState& modProfile, uint8_t cAngleSlot, bool modX = false, bool modY = false)
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

void applyDirectionalOutput(GamepadState& state, uint8_t dpadMask, DpadMode dpadMode, const GlyphProfiles::ModProfileState& modProfile, uint8_t cAngleSlot, bool modX, bool modY)
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
    GamepadState& state,
    LegacyPlatformProfile profile,
    const GlyphProfiles::ModProfileState& modProfile,
    uint8_t buttonId,
    bool dpadLayer,
    uint8_t& dpadOutput,
    uint8_t& leftAnalogOutput,
    uint8_t& rightAnalogOutput
) {
    auto setDigitalTriggerL = [&]() {
        state.buttons |= GAMEPAD_MASK_L2;
        if (state.lt < 140) {
            state.lt = 140;
        }
        return true;
    };

    auto setDigitalTriggerR = [&]() {
        state.buttons |= GAMEPAD_MASK_R2;
        if (state.rt < 140) {
            state.rt = 140;
        }
        return true;
    };

    switch (buttonId) {
        case kGlyphButtonMB7:
            state.buttons |= GAMEPAD_MASK_S2;
            return true;
        case kGlyphButtonMB6:
            state.buttons |= GAMEPAD_MASK_S1;
            return true;
        case kGlyphButtonMB5:
            state.aux |= AUX_MASK_FUNCTION;
            return true;
        case kGlyphButtonMB4:
            state.buttons |= GAMEPAD_MASK_A2;
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
                        state.buttons |= GAMEPAD_MASK_L3;
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
                        state.buttons |= GAMEPAD_MASK_R3;
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
                    if (profile == LegacyPlatformProfile::Rivals) {
                        state.buttons |= GAMEPAD_MASK_L1;
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
                        state.buttons |= GAMEPAD_MASK_L1;
                        return true;
                    }
                    return false;

                case kGlyphButtonRT1: state.buttons |= GAMEPAD_MASK_B1; return true;
                case kGlyphButtonRF1: state.buttons |= GAMEPAD_MASK_B2; return true;
                case kGlyphButtonRF2: state.buttons |= GAMEPAD_MASK_B3; return true;
                case kGlyphButtonRF6: state.buttons |= GAMEPAD_MASK_B4; return true;
                case kGlyphButtonRF3: state.buttons |= GAMEPAD_MASK_R1; return true;
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

bool GlyphMatrixInput::available()
{
    return GLYPH_MATRIX_INPUT_ENABLED == 1;
}

void GlyphMatrixInput::setup()
{
    ensureProfiles();

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

bool GlyphMatrixInput::glyphModXPressed()
{
    return glyphModXPressedState;
}

bool GlyphMatrixInput::glyphModYPressed()
{
    return glyphModYPressedState;
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

void GlyphMatrixInput::apply(GamepadState& state)
{
    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    const uint8_t profile = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;
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
    bool modXPressed = false;
    bool modYPressed = false;
    uint8_t cAngleSlot = kNoCAngleSlot;

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
                                 (remap.activates < sizeof(remappedGlyphPressed) && remappedGlyphPressed[remap.activates]);
        remappedPhysical[remap.physicalButton] = true;
        if (remap.activates > 0 && remap.activates < sizeof(remappedGlyphPressed)) {
            remappedGlyphPressed[remap.activates] = shouldPress;
            if (glyphButtonPressedAt[remap.activates] == 0) {
                glyphButtonPressedAt[remap.activates] = glyphButtonPressedAt[remap.physicalButton];
            }
        }
    }

    for (uint8_t buttonId = 1; buttonId < sizeof(glyphPressed); buttonId++) {
        glyphPressed[buttonId] = remappedGlyphPressed[buttonId] ||
                                 (originalGlyphPressed[buttonId] && !remappedPhysical[buttonId]);
    }

    for (uint8_t pairIndex = 0; pairIndex < GlyphProfiles::socdPairCount(profile); pairIndex++) {
        const GlyphProfiles::SocdPair& pair = GlyphProfiles::socdPair(profile, pairIndex);
        if (pair.buttonDir1 >= sizeof(glyphPressed) || pair.buttonDir2 >= sizeof(glyphPressed)) {
            continue;
        }
        if (!glyphPressed[pair.buttonDir1] || !glyphPressed[pair.buttonDir2]) {
            continue;
        }

        switch (pair.socdType) {
            case 1: // Neutral
                glyphPressed[pair.buttonDir1] = false;
                glyphPressed[pair.buttonDir2] = false;
                break;
            case 4: // Dir1 priority
                glyphPressed[pair.buttonDir2] = false;
                break;
            case 5: // Dir2 priority
                glyphPressed[pair.buttonDir1] = false;
                break;
            case 2: // 2IP
            case 3: // 2IP no-react, approximated as 2IP for now
            default:
                if (glyphButtonPressedAt[pair.buttonDir1] <= glyphButtonPressedAt[pair.buttonDir2]) {
                    glyphPressed[pair.buttonDir1] = false;
                } else {
                    glyphPressed[pair.buttonDir2] = false;
                }
                break;
        }
    }

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
                modXPressed = true;
                continue;
            }
            if (buttonId == kGlyphButtonLT2) {
                modYPressed = true;
                continue;
            }
        }

        if (legacyPlatform != LegacyPlatformProfile::None &&
            applyLegacyPlatformButton(
                state,
                legacyPlatform,
                modProfile,
                buttonId,
                modXPressed && modYPressed,
                dpadOutput,
                leftAnalogOutput,
                rightAnalogOutput
            )) {
            continue;
        }

        const GlyphProfiles::Action action = GlyphProfiles::buttonAction(profile, buttonId);
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

    if (legacyPlatform != LegacyPlatformProfile::None && leftAnalogOutput != 0 && cAngleSlot == kNoCAngleSlot &&
        (modXPressed || modYPressed)) {
        applyLegacyLeftStickModifier(state, leftAnalogOutput, legacyPlatform, modProfile, modXPressed, modYPressed);
        leftAnalogOutput = 0;
    }

    if (gamepad != nullptr) {
        applyDirectionalOutput(state, dpadOutput, gamepad->getOptions().dpadMode, modProfile, cAngleSlot, modXPressed, modYPressed);
    }
    applyAnalogOutput(state, leftAnalogOutput, false, modProfile, cAngleSlot, modXPressed, modYPressed);
    applyAnalogOutput(state, rightAnalogOutput, true, modProfile, cAngleSlot, modXPressed, modYPressed);
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
            EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(actions[i]));
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
