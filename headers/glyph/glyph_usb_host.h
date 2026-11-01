#ifndef _GLYPH_USB_HOST_H_
#define _GLYPH_USB_HOST_H_

#include "config.pb.h"
#include "enums.pb.h"

#include <stdint.h>

namespace GlyphUsbHost
{
constexpr int32_t OptionXInputAuth = 1;
constexpr int32_t OptionXBOneAuth = 2;
constexpr int32_t OptionPS4Auth = 4;
constexpr int32_t OptionPS5Auth = 8;
constexpr int32_t OptionDisable = 32;

void sanitizeConfig(Config& config);
bool toggleOption(AddonOptions& addonOptions, GamepadOptions& gamepadOptions, PeripheralOptions& peripheralOptions, int32_t option);
int32_t currentOption(const GamepadOptions& gamepadOptions, const PeripheralOptions& peripheralOptions, int32_t option);
}

#endif
