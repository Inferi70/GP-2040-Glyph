#include "addons/glyph_matrix_input.h"

#include "eventmanager.h"
#include "gamepad.h"
#include "glyph/glyph_profiles.h"
#include "storagemanager.h"
#include "types.h"

#include "pico/stdlib.h"

#include <cstring>

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
constexpr uint8_t kGlyphButtonLT1 = 33;
constexpr uint8_t kGlyphButtonLT2 = 34;
constexpr uint8_t kFullAnalogMagnitude = 127;
constexpr uint8_t kModXHorizontalMagnitude = 53;
constexpr uint8_t kModXVerticalMagnitude = 43;
constexpr uint8_t kModYHorizontalMagnitude = 27;
constexpr uint8_t kModYVerticalMagnitude = 59;

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

void applyAnalogOutput(GamepadState& state, uint8_t mask, bool rightStick, bool modX = false, bool modY = false)
{
    uint16_t& x = rightStick ? state.rx : state.lx;
    uint16_t& y = rightStick ? state.ry : state.ly;
    uint8_t xMagnitude = kFullAnalogMagnitude;
    uint8_t yMagnitude = kFullAnalogMagnitude;

    if (modX) {
        xMagnitude = kModXHorizontalMagnitude;
        yMagnitude = kModXVerticalMagnitude;
    }
    if (modY) {
        xMagnitude = kModYHorizontalMagnitude;
        yMagnitude = kModYVerticalMagnitude;
    }

    if (mask & GAMEPAD_MASK_LEFT) x = analogValue(false, xMagnitude);
    if (mask & GAMEPAD_MASK_RIGHT) x = analogValue(true, xMagnitude);
    if (mask & GAMEPAD_MASK_UP) y = analogValue(false, yMagnitude);
    if (mask & GAMEPAD_MASK_DOWN) y = analogValue(true, yMagnitude);
}

void applyDirectionalOutput(GamepadState& state, uint8_t dpadMask, DpadMode dpadMode, bool modX, bool modY)
{
    // GP2040 stick mode only redirects the main dpad target. Dedicated LS/RS
    // targets are handled separately and always write that stick directly.
    switch (dpadMode) {
        case DpadMode::DPAD_MODE_LEFT_ANALOG:
            applyAnalogOutput(state, dpadMask, false, modX, modY);
            break;
        case DpadMode::DPAD_MODE_RIGHT_ANALOG:
            applyAnalogOutput(state, dpadMask, true, modX, modY);
            break;
        case DpadMode::DPAD_MODE_DIGITAL:
        default:
            state.dpad |= dpadMask;
            break;
    }
}
}

static GlyphMatrixInput::DebounceState debounce[kRows][kCols] = {};
static bool pressed[kRows][kCols] = {};
static bool menuControlHeld[4] = {};
static uint32_t menuControlLastSent[4] = {};
static uint32_t glyphButtonPressedAt[61] = {};
static bool glyphButtonPressedState[61] = {};
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
    bool originalGlyphPressed[61] = {};
    bool remappedGlyphPressed[61] = {};
    bool glyphPressed[61] = {};
    bool remappedPhysical[61] = {};
    uint8_t dpadOutput = 0;
    uint8_t leftAnalogOutput = 0;
    uint8_t rightAnalogOutput = 0;
    bool modXPressed = false;
    bool modYPressed = false;

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

    for (uint8_t buttonId = 1; buttonId < sizeof(glyphPressed); buttonId++) {
        if (!glyphPressed[buttonId]) {
            continue;
        }

        // FW-Glyph treats LT1/LT2 as internal analog modifiers in platform
        // layouts; they are not emitted as controller buttons.
        if (GlyphProfiles::layout(profile) == GlyphProfiles::Layout::Platform) {
            if (buttonId == kGlyphButtonLT1) {
                modXPressed = true;
                continue;
            }
            if (buttonId == kGlyphButtonLT2) {
                modYPressed = true;
                continue;
            }
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

    if (gamepad != nullptr) {
        applyDirectionalOutput(state, dpadOutput, gamepad->getOptions().dpadMode, modXPressed, modYPressed);
    }
    applyAnalogOutput(state, leftAnalogOutput, false);
    applyAnalogOutput(state, rightAnalogOutput, true);
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
