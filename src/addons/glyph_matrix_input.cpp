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

void setProfileLabel(char* dest, size_t len, const char* label)
{
    strncpy(dest, label, len - 1);
    dest[len - 1] = '\0';
}
}

static GlyphMatrixInput::DebounceState debounce[kRows][kCols] = {};
static bool pressed[kRows][kCols] = {};
static bool menuControlHeld[4] = {};
static uint32_t menuControlLastSent[4] = {};

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

    if (config.profileOptions.gpioMappingsSets_count < GlyphProfiles::count() - 1) {
        config.profileOptions.gpioMappingsSets_count = GlyphProfiles::count() - 1;
    }

    for (uint8_t profile = 0; profile < GlyphProfiles::count() - 1; profile++) {
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

    Gamepad* gamepad = Storage::getInstance().GetGamepad();
    if (gamepad != nullptr) {
        applyProfileOptions();
        apply(gamepad->state);
    }
}

void GlyphMatrixInput::applyProfileOptions()
{
    Storage& storage = Storage::getInstance();
    GamepadOptions& options = storage.getGamepadOptions();
    const SOCDMode profileSocd = GlyphProfiles::socdMode(options.profileNumber);

    if (options.socdMode != profileSocd) {
        options.socdMode = profileSocd;
    }
}

void GlyphMatrixInput::process()
{
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

    for (uint8_t row = 0; row < kRows; row++) {
        for (uint8_t col = 0; col < kCols; col++) {
            if (!pressed[row][col]) {
                continue;
            }

            const GlyphProfiles::Action action = GlyphProfiles::matrixAction(profile, row, col);
            switch (action.target) {
                case GlyphProfiles::Target::Dpad:
                    state.dpad |= action.mask;
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
    }
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
