#include "glyph/glyph_profiles.h"

#include "gamepad/GamepadState.h"

#include <string.h>

namespace
{
using GlyphProfiles::BackendDInput;
using GlyphProfiles::BackendGameCube;
using GlyphProfiles::BackendN64;
using GlyphProfiles::BackendPS4;
using GlyphProfiles::BackendPS5;
using GlyphProfiles::BackendSwitch;
using GlyphProfiles::BackendXInput;
using GlyphProfiles::Action;
using GlyphProfiles::Layout;
using GlyphProfiles::OutputIcon;
using GlyphProfiles::Profile;
using GlyphProfiles::Target;

constexpr uint16_t kModernBackends = BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5;
constexpr uint16_t kPlatformBackends = kModernBackends | BackendGameCube;

constexpr Action none()
{
    return {Target::None, 0};
}

constexpr Action dpad(uint32_t mask)
{
    return {Target::Dpad, mask};
}

constexpr Action button(uint32_t mask)
{
    return {Target::Button, mask};
}

constexpr Action aux(uint32_t mask)
{
    return {Target::Aux, mask};
}

constexpr Profile kProfiles[] = {
    {1, "Melee",     Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 1, kPlatformBackends},
    {2, "Brawl",     Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 2, kPlatformBackends},
    {3, "Ultimate",  Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 3, kPlatformBackends},
    {4, "Split FGC", Layout::SplitFgc, SOCD_MODE_NEUTRAL,               4, kModernBackends},
    {5, "FGC",       Layout::Fgc,      SOCD_MODE_NEUTRAL,               5, kModernBackends},
    {6, "Smash64",   Layout::Platform, SOCD_MODE_NEUTRAL,               6, BackendN64},
};

constexpr Action kPlatformMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {button(GAMEPAD_MASK_E1),  button(GAMEPAD_MASK_E2),  button(GAMEPAD_MASK_E3),  button(GAMEPAD_MASK_E4),  aux(AUX_MASK_FUNCTION), button(GAMEPAD_MASK_S1), button(GAMEPAD_MASK_S2), button(GAMEPAD_MASK_L2), button(GAMEPAD_MASK_E9), none(), none()},
    {dpad(GAMEPAD_MASK_LEFT),  dpad(GAMEPAD_MASK_DOWN),  button(GAMEPAD_MASK_E8),  button(GAMEPAD_MASK_E7),  button(GAMEPAD_MASK_E10), button(GAMEPAD_MASK_E11), button(GAMEPAD_MASK_E12), button(GAMEPAD_MASK_B3), button(GAMEPAD_MASK_B4), button(GAMEPAD_MASK_R1), button(GAMEPAD_MASK_L1)},
    {button(GAMEPAD_MASK_E5),  button(GAMEPAD_MASK_E6),  dpad(GAMEPAD_MASK_RIGHT), dpad(GAMEPAD_MASK_UP),    button(GAMEPAD_MASK_E10), button(GAMEPAD_MASK_E11), button(GAMEPAD_MASK_E12), button(GAMEPAD_MASK_B1), button(GAMEPAD_MASK_B2), button(GAMEPAD_MASK_R2), dpad(GAMEPAD_MASK_UP)},
    {button(GAMEPAD_MASK_A3),  button(GAMEPAD_MASK_A4),  button(GAMEPAD_MASK_A1),  button(GAMEPAD_MASK_A2),  button(GAMEPAD_MASK_L3), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_E2), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_E3), button(GAMEPAD_MASK_E4)},
};

constexpr Action kFgcMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {none(),                  none(),                  none(),                  none(),                  aux(AUX_MASK_FUNCTION), button(GAMEPAD_MASK_S1), button(GAMEPAD_MASK_S2), button(GAMEPAD_MASK_R2), none(),                  none(),                 none()},
    {none(),                  none(),                  dpad(GAMEPAD_MASK_LEFT), dpad(GAMEPAD_MASK_DOWN), button(GAMEPAD_MASK_B4), button(GAMEPAD_MASK_L1), button(GAMEPAD_MASK_R1), button(GAMEPAD_MASK_L2), none(),                  none(),                 none()},
    {none(),                  none(),                  none(),                  dpad(GAMEPAD_MASK_RIGHT), dpad(GAMEPAD_MASK_UP),  button(GAMEPAD_MASK_B1), button(GAMEPAD_MASK_B2), button(GAMEPAD_MASK_B3), none(),                  none(),                 none()},
    {none(),                  none(),                  none(),                  none(),                  none(),                  button(GAMEPAD_MASK_A1), none(),                 none(),                 none(),                  none(),                 none()},
};

constexpr Action kSplitFgcMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {none(),                  none(),                  none(),                  none(),                  aux(AUX_MASK_FUNCTION), button(GAMEPAD_MASK_S1), button(GAMEPAD_MASK_S2), none(),                 button(GAMEPAD_MASK_R3), none(),                 none()},
    {dpad(GAMEPAD_MASK_LEFT), dpad(GAMEPAD_MASK_DOWN), none(),                  none(),                  none(),                 none(),                 none(),                 button(GAMEPAD_MASK_L2), button(GAMEPAD_MASK_B4), button(GAMEPAD_MASK_L1), button(GAMEPAD_MASK_R2)},
    {none(),                  button(GAMEPAD_MASK_L3), dpad(GAMEPAD_MASK_RIGHT), none(),                 dpad(GAMEPAD_MASK_UP),   none(),                 none(),                 button(GAMEPAD_MASK_B3), button(GAMEPAD_MASK_B1), button(GAMEPAD_MASK_B2), button(GAMEPAD_MASK_R1)},
    {none(),                  none(),                  button(GAMEPAD_MASK_A1), none(),                  none(),                  none(),                 none(),                 none(),                 button(GAMEPAD_MASK_R3), none(),                 none()},
};

constexpr OutputIcon kMenuButtonIcons[][7] = {
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
};

constexpr const char* kBackendSummaries[] = {
    "",
    "X/D/SW/PS/GC",
    "X/D/SW/PS/GC",
    "X/D/SW/PS/GC",
    "X/D/SW/PS",
    "X/D/SW/PS",
    "N64",
};

constexpr uint32_t kGlyphOptionsVersion = 1;

GlyphProfiles::ProfileState mutableProfiles[GlyphProfiles::MaxProfiles] = {};
bool mutableProfilesReady = false;

void copyName(char* destination, const char* source)
{
    if (destination == nullptr) {
        return;
    }

    if (source == nullptr) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, GlyphProfiles::ProfileNameLength);
    destination[GlyphProfiles::ProfileNameLength] = '\0';
}

void ensureMutableProfiles()
{
    if (mutableProfilesReady) {
        return;
    }

    for (uint8_t i = 0; i < GlyphProfiles::count(); i++) {
        mutableProfiles[i].number = kProfiles[i].number;
        copyName(mutableProfiles[i].name, kProfiles[i].name);
        mutableProfiles[i].layout = kProfiles[i].layout;
        mutableProfiles[i].socdMode = kProfiles[i].socdMode;
        mutableProfiles[i].rgbConfig = kProfiles[i].rgbConfig;
        mutableProfiles[i].backends = kProfiles[i].backends;
    }
    mutableProfilesReady = true;
}

bool validLayout(uint32_t value)
{
    return value <= static_cast<uint32_t>(Layout::SplitFgc);
}

uint8_t clampProfile(uint8_t profileNumber)
{
    return (profileNumber >= 1 && profileNumber <= GlyphProfiles::count()) ? profileNumber : 1;
}

const Action (&matrixForLayout(Layout layout))[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols]
{
    switch (layout) {
        case Layout::Fgc:
            return kFgcMatrix;
        case Layout::SplitFgc:
            return kSplitFgcMatrix;
        case Layout::Platform:
        default:
            return kPlatformMatrix;
    }
}
}

namespace GlyphProfiles
{
uint8_t count()
{
    return sizeof(kProfiles) / sizeof(kProfiles[0]);
}

const Profile& get(uint8_t profileNumber)
{
    return kProfiles[clampProfile(profileNumber) - 1];
}

const ProfileState& state(uint8_t profileNumber)
{
    ensureMutableProfiles();
    return mutableProfiles[clampProfile(profileNumber) - 1];
}

const char* name(uint8_t profileNumber)
{
    return state(profileNumber).name;
}

Layout layout(uint8_t profileNumber)
{
    return state(profileNumber).layout;
}

SOCDMode socdMode(uint8_t profileNumber)
{
    return state(profileNumber).socdMode;
}

uint8_t rgbConfig(uint8_t profileNumber)
{
    return state(profileNumber).rgbConfig;
}

uint16_t backends(uint8_t profileNumber)
{
    return state(profileNumber).backends;
}

void resetToDefaults()
{
    mutableProfilesReady = false;
    ensureMutableProfiles();
}

void loadFromConfig(const GlyphOptions& options)
{
    resetToDefaults();

    const uint8_t profileCount = options.profiles_count < count() ? options.profiles_count : count();
    for (uint8_t i = 0; i < profileCount; i++) {
        const GlyphProfileEntry& source = options.profiles[i];
        ProfileState& destination = mutableProfiles[i];

        if (source.name[0] != '\0') {
            copyName(destination.name, source.name);
        }
        if (validLayout(source.layout)) {
            destination.layout = static_cast<Layout>(source.layout);
        }
        destination.socdMode = source.socdMode;
        destination.rgbConfig = static_cast<uint8_t>(source.rgbConfig);
        destination.backends = static_cast<uint16_t>(source.backends);
    }
}

void writeToConfig(GlyphOptions& options)
{
    ensureMutableProfiles();

    options.version = kGlyphOptionsVersion;
    options.profiles_count = count();
    for (uint8_t i = 0; i < count(); i++) {
        GlyphProfileEntry& destination = options.profiles[i];
        const ProfileState& source = mutableProfiles[i];

        copyName(destination.name, source.name);
        destination.layout = static_cast<uint32_t>(source.layout);
        destination.socdMode = source.socdMode;
        destination.rgbConfig = source.rgbConfig;
        destination.backends = source.backends;
    }
}

void setName(uint8_t profileNumber, const char* value)
{
    ensureMutableProfiles();
    copyName(mutableProfiles[clampProfile(profileNumber) - 1].name, value);
}

void setLayout(uint8_t profileNumber, Layout value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].layout = value;
}

void setSOCDMode(uint8_t profileNumber, SOCDMode value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].socdMode = value;
}

void setRgbConfig(uint8_t profileNumber, uint8_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].rgbConfig = value;
}

void setBackends(uint8_t profileNumber, uint16_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].backends = value;
}

bool backendEnabled(uint8_t profileNumber, uint16_t backendMask)
{
    return (backends(profileNumber) & backendMask) != 0;
}

void setBackendEnabled(uint8_t profileNumber, uint16_t backendMask, bool enabled)
{
    ensureMutableProfiles();
    uint16_t& mask = mutableProfiles[clampProfile(profileNumber) - 1].backends;
    if (enabled) {
        mask |= backendMask;
    } else {
        mask &= ~backendMask;
    }
}

bool allowsInputMode(uint8_t profileNumber, InputMode mode)
{
    const uint16_t mask = backends(profileNumber);

    switch (mode) {
        case INPUT_MODE_XINPUT:
        case INPUT_MODE_XBONE:
            return (mask & BackendXInput) != 0;
        case INPUT_MODE_SWITCH:
        case INPUT_MODE_SWITCH_PRO:
            return (mask & BackendSwitch) != 0;
        case INPUT_MODE_PS4:
            return (mask & BackendPS4) != 0;
        case INPUT_MODE_PS5:
        case INPUT_MODE_P5GENERAL:
            return (mask & BackendPS5) != 0;
        default:
            return (mask & BackendDInput) != 0;
    }
}

const char* backendSummary(uint8_t profileNumber)
{
    const uint16_t mask = backends(profileNumber);
    if (mask == BackendN64) {
        return "N64";
    }
    if ((mask & (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5 | BackendGameCube)) ==
        (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5 | BackendGameCube)) {
        return "X/D/SW/PS/GC";
    }
    if ((mask & (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5)) ==
        (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5)) {
        return "X/D/SW/PS";
    }
    return kBackendSummaries[clampProfile(profileNumber)];
}

const char* layoutName(Layout layout)
{
    switch (layout) {
        case Layout::Fgc: return "FGC";
        case Layout::SplitFgc: return "Split FGC";
        case Layout::Platform:
        default:
            return "Full";
    }
}

OutputIcon menuIcon(uint8_t profileNumber, uint8_t menuButtonIndex)
{
    if (menuButtonIndex >= 7) {
        return OutputIcon::None;
    }

    return kMenuButtonIcons[clampProfile(profileNumber) - 1][menuButtonIndex];
}

Action matrixAction(uint8_t profileNumber, uint8_t row, uint8_t col)
{
    if (row >= MatrixRows || col >= MatrixCols) {
        return {Target::None, 0};
    }

    const Action (&matrix)[MatrixRows][MatrixCols] = matrixForLayout(layout(profileNumber));
    return matrix[row][col];
}
}
