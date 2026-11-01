#include "glyph/glyph_usb_host.h"

#include "BoardConfig.h"

#ifndef USB_PERIPHERAL_PIN_5V
#define USB_PERIPHERAL_PIN_5V -1
#endif

namespace GlyphUsbHost
{
namespace
{
bool isSupportedOption(int32_t option)
{
    return option == OptionXInputAuth || option == OptionXBOneAuth || option == OptionPS4Auth || option == OptionPS5Auth || option == OptionDisable;
}

bool hasSupportedAuth(const GamepadOptions& gamepadOptions)
{
    return gamepadOptions.xinputAuthType == INPUT_MODE_AUTH_TYPE_USB ||
           gamepadOptions.ps4AuthType == INPUT_MODE_AUTH_TYPE_USB ||
           gamepadOptions.ps5AuthType == INPUT_MODE_AUTH_TYPE_USB;
}

void configurePort(PeripheralOptions& peripheralOptions)
{
    peripheralOptions.blockUSB0.enabled = true;
    peripheralOptions.blockUSB0.dp = USB_PERIPHERAL_PIN_DPLUS;
    peripheralOptions.blockUSB0.order = USB_PERIPHERAL_PIN_ORDER;
    peripheralOptions.blockUSB0.enable5v = USB_PERIPHERAL_PIN_5V;
}

void disableAll(AddonOptions& addonOptions, GamepadOptions& gamepadOptions, PeripheralOptions& peripheralOptions)
{
    peripheralOptions.blockUSB0.enabled = false;
    addonOptions.gamepadUSBHostOptions.enabled = false;
    addonOptions.keyboardHostOptions.enabled = false;
    gamepadOptions.xinputAuthType = INPUT_MODE_AUTH_TYPE_NONE;
    gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_NONE;
    gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_NONE;
}
}

void sanitizeConfig(Config& config)
{
    AddonOptions& addonOptions = config.addonOptions;
    GamepadOptions& gamepadOptions = config.gamepadOptions;
    PeripheralOptions& peripheralOptions = config.peripheralOptions;

    addonOptions.gamepadUSBHostOptions.enabled = false;
    addonOptions.keyboardHostOptions.enabled = false;

    // Glyph only configures the physical USB port here. Auth implementations stay upstream GP2040.
    if (hasSupportedAuth(gamepadOptions) || peripheralOptions.blockUSB0.enabled) {
        configurePort(peripheralOptions);
    } else {
        peripheralOptions.blockUSB0.enabled = false;
    }
}

bool toggleOption(AddonOptions& addonOptions, GamepadOptions& gamepadOptions, PeripheralOptions& peripheralOptions, int32_t option)
{
    if (!isSupportedOption(option)) {
        return false;
    }

    if (option == OptionDisable) {
        disableAll(addonOptions, gamepadOptions, peripheralOptions);
        return true;
    }

    addonOptions.gamepadUSBHostOptions.enabled = false;
    addonOptions.keyboardHostOptions.enabled = false;

    if (option == OptionXInputAuth) {
        const bool enableAuth = gamepadOptions.xinputAuthType != INPUT_MODE_AUTH_TYPE_USB;
        gamepadOptions.xinputAuthType = enableAuth ? INPUT_MODE_AUTH_TYPE_USB : INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        peripheralOptions.blockUSB0.enabled = enableAuth;
    } else if (option == OptionXBOneAuth) {
        const bool enablePort = hasSupportedAuth(gamepadOptions) || !peripheralOptions.blockUSB0.enabled;
        gamepadOptions.xinputAuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        peripheralOptions.blockUSB0.enabled = enablePort;
    } else if (option == OptionPS4Auth) {
        const bool enableAuth = gamepadOptions.ps4AuthType != INPUT_MODE_AUTH_TYPE_USB;
        gamepadOptions.xinputAuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps4AuthType = enableAuth ? INPUT_MODE_AUTH_TYPE_USB : INPUT_MODE_AUTH_TYPE_NONE;
        peripheralOptions.blockUSB0.enabled = enableAuth;
    } else if (option == OptionPS5Auth) {
        const bool enableAuth = gamepadOptions.ps5AuthType != INPUT_MODE_AUTH_TYPE_USB;
        gamepadOptions.xinputAuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_NONE;
        gamepadOptions.ps5AuthType = enableAuth ? INPUT_MODE_AUTH_TYPE_USB : INPUT_MODE_AUTH_TYPE_NONE;
        peripheralOptions.blockUSB0.enabled = enableAuth;
    }

    if (hasSupportedAuth(gamepadOptions) || peripheralOptions.blockUSB0.enabled) {
        configurePort(peripheralOptions);
    } else {
        peripheralOptions.blockUSB0.enabled = false;
    }

    return true;
}

int32_t currentOption(const GamepadOptions& gamepadOptions, const PeripheralOptions& peripheralOptions, int32_t option)
{
    switch (option) {
        case OptionXInputAuth:
            return gamepadOptions.xinputAuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case OptionXBOneAuth:
            return peripheralOptions.blockUSB0.enabled && !hasSupportedAuth(gamepadOptions) ? option : 0;
        case OptionPS4Auth:
            return gamepadOptions.ps4AuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case OptionPS5Auth:
            return gamepadOptions.ps5AuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case OptionDisable:
            return 0;
        default:
            return 0;
    }
}
}
