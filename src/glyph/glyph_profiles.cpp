#include "glyph/glyph_profiles.h"

#include "gamepad/GamepadState.h"

#include <stdio.h>
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
constexpr uint8_t BTN_UNSPECIFIED = 0;
constexpr uint8_t BTN_LF1 = 1;
constexpr uint8_t BTN_LF2 = 2;
constexpr uint8_t BTN_LF3 = 3;
constexpr uint8_t BTN_LF4 = 4;
constexpr uint8_t BTN_LF5 = 5;
constexpr uint8_t BTN_LF6 = 6;
constexpr uint8_t BTN_LF7 = 7;
constexpr uint8_t BTN_LF8 = 8;
constexpr uint8_t BTN_RF1 = 17;
constexpr uint8_t BTN_RF2 = 18;
constexpr uint8_t BTN_RF3 = 19;
constexpr uint8_t BTN_RF4 = 20;
constexpr uint8_t BTN_RF5 = 21;
constexpr uint8_t BTN_RF6 = 22;
constexpr uint8_t BTN_RF7 = 23;
constexpr uint8_t BTN_RF8 = 24;
constexpr uint8_t BTN_RF9 = 25;
constexpr uint8_t BTN_RF10 = 26;
constexpr uint8_t BTN_RF11 = 27;
constexpr uint8_t BTN_RF12 = 28;
constexpr uint8_t BTN_RF13 = 29;
constexpr uint8_t BTN_RF14 = 30;
constexpr uint8_t BTN_RF15 = 31;
constexpr uint8_t BTN_RF16 = 32;
constexpr uint8_t BTN_LT1 = 33;
constexpr uint8_t BTN_LT2 = 34;
constexpr uint8_t BTN_LT3 = 35;
constexpr uint8_t BTN_LT4 = 36;
constexpr uint8_t BTN_LT5 = 37;
constexpr uint8_t BTN_LT6 = 38;
constexpr uint8_t BTN_RT1 = 41;
constexpr uint8_t BTN_RT2 = 42;
constexpr uint8_t BTN_RT3 = 43;
constexpr uint8_t BTN_RT4 = 44;
constexpr uint8_t BTN_RT5 = 45;
constexpr uint8_t BTN_MB1 = 49;
constexpr uint8_t BTN_MB2 = 50;
constexpr uint8_t BTN_MB3 = 51;
constexpr uint8_t BTN_MB4 = 52;
constexpr uint8_t BTN_MB5 = 53;
constexpr uint8_t BTN_MB6 = 54;
constexpr uint8_t BTN_MB7 = 55;
constexpr uint8_t SOCD_NEUTRAL = 1;
constexpr uint8_t SOCD_2IP = 2;
constexpr uint8_t SOCD_2IP_NO_REAC = 3;
constexpr uint32_t GLYPH_DEFAULT_RGB_COLOR = 2282478;

constexpr Action none()
{
    return {Target::None, 0};
}

constexpr Action dpad(uint32_t mask)
{
    return {Target::Dpad, mask};
}

constexpr Action leftAnalog(uint32_t mask)
{
    return {Target::LeftAnalog, mask};
}

constexpr Action rightAnalog(uint32_t mask)
{
    return {Target::RightAnalog, mask};
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
constexpr uint8_t kPresetProfileCount = sizeof(kProfiles) / sizeof(kProfiles[0]);

constexpr GlyphProfiles::SocdPair kPlatformSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_2IP_NO_REAC},
    {BTN_LF2, BTN_RF4, SOCD_2IP_NO_REAC},
    {BTN_RT3, BTN_RT5, SOCD_2IP_NO_REAC},
    {BTN_RT2, BTN_RT4, SOCD_2IP_NO_REAC},
};

constexpr GlyphProfiles::SocdPair kUltimateSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_2IP},
    {BTN_LF2, BTN_RF4, SOCD_2IP},
    {BTN_RT3, BTN_RT5, SOCD_2IP},
    {BTN_RT2, BTN_RT4, SOCD_2IP},
};

constexpr GlyphProfiles::SocdPair kFgcSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_NEUTRAL},
    {BTN_LF2, BTN_LT1, SOCD_NEUTRAL},
};

constexpr GlyphProfiles::ButtonRemap kMeleeRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_LF8, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LF6, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kBrawlRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_LF8, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LF6, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
    {BTN_RF9, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kSplitFgcRemaps[] = {
    {BTN_RT1, BTN_LT1},
    {BTN_LF5, BTN_LT2},
    {BTN_RF9, BTN_RT1},
    {BTN_LF8, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LF6, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
    {BTN_LT2, BTN_UNSPECIFIED},
    {BTN_RT2, BTN_UNSPECIFIED},
    {BTN_RT3, BTN_UNSPECIFIED},
    {BTN_RT4, BTN_UNSPECIFIED},
    {BTN_RT5, BTN_UNSPECIFIED},
    {BTN_MB1, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kFgcRemaps[] = {
    {BTN_RF10, BTN_RF1},
    {BTN_RF11, BTN_RF2},
    {BTN_RF12, BTN_RF3},
    {BTN_RF1, BTN_RF4},
    {BTN_RF13, BTN_RF5},
    {BTN_RF14, BTN_RF6},
    {BTN_RF15, BTN_RF7},
    {BTN_RF5, BTN_RF8},
    {BTN_LF6, BTN_LF1},
    {BTN_LF7, BTN_LF2},
    {BTN_LF8, BTN_LF3},
    {BTN_LT6, BTN_LT1},
    {BTN_RF16, BTN_LT2},
    {BTN_RF2, BTN_UNSPECIFIED},
    {BTN_RF3, BTN_UNSPECIFIED},
    {BTN_RF4, BTN_UNSPECIFIED},
    {BTN_RF6, BTN_UNSPECIFIED},
    {BTN_RF7, BTN_UNSPECIFIED},
    {BTN_RF8, BTN_UNSPECIFIED},
    {BTN_LF1, BTN_UNSPECIFIED},
    {BTN_LF2, BTN_UNSPECIFIED},
    {BTN_LF3, BTN_UNSPECIFIED},
    {BTN_LF5, BTN_UNSPECIFIED},
    {BTN_LT1, BTN_UNSPECIFIED},
    {BTN_LT2, BTN_UNSPECIFIED},
    {BTN_RT1, BTN_UNSPECIFIED},
    {BTN_RT2, BTN_UNSPECIFIED},
    {BTN_RT3, BTN_UNSPECIFIED},
    {BTN_RT4, BTN_UNSPECIFIED},
    {BTN_RT5, BTN_UNSPECIFIED},
    {BTN_MB1, BTN_UNSPECIFIED},
};

constexpr Action kPlatformMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {button(GAMEPAD_MASK_E1),  button(GAMEPAD_MASK_E2),  button(GAMEPAD_MASK_E3),  button(GAMEPAD_MASK_E4),  aux(AUX_MASK_FUNCTION), button(GAMEPAD_MASK_S1), button(GAMEPAD_MASK_S2), button(GAMEPAD_MASK_L2), button(GAMEPAD_MASK_E9), none(), none()},
    {dpad(GAMEPAD_MASK_LEFT),  dpad(GAMEPAD_MASK_DOWN),  button(GAMEPAD_MASK_E8),  button(GAMEPAD_MASK_E7),  button(GAMEPAD_MASK_E10), button(GAMEPAD_MASK_E11), button(GAMEPAD_MASK_E12), button(GAMEPAD_MASK_B3), button(GAMEPAD_MASK_B4), button(GAMEPAD_MASK_R1), button(GAMEPAD_MASK_L1)},
    {button(GAMEPAD_MASK_E5),  button(GAMEPAD_MASK_E6),  dpad(GAMEPAD_MASK_RIGHT), dpad(GAMEPAD_MASK_UP),    button(GAMEPAD_MASK_E10), button(GAMEPAD_MASK_E11), button(GAMEPAD_MASK_E12), button(GAMEPAD_MASK_B1), button(GAMEPAD_MASK_B2), button(GAMEPAD_MASK_R2), dpad(GAMEPAD_MASK_UP)},
    {button(GAMEPAD_MASK_A3),  button(GAMEPAD_MASK_A4),  button(GAMEPAD_MASK_A1),  button(GAMEPAD_MASK_A2),  button(GAMEPAD_MASK_L3), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_E2), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_R3), button(GAMEPAD_MASK_E3), button(GAMEPAD_MASK_E4)},
};

constexpr uint8_t kGlyphButtonMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {BTN_MB1, BTN_MB2, BTN_MB3, BTN_MB4, BTN_MB5, BTN_MB6, BTN_MB7, BTN_RF16, BTN_RF9, BTN_UNSPECIFIED, BTN_UNSPECIFIED},
    {BTN_LF3, BTN_LF2, BTN_LF8, BTN_LF7, BTN_RF13, BTN_RF14, BTN_RF15, BTN_RF5, BTN_RF6, BTN_RF7, BTN_RF8},
    {BTN_LF4, BTN_LF5, BTN_LF1, BTN_LF6, BTN_RF10, BTN_RF11, BTN_RF12, BTN_RF1, BTN_RF2, BTN_RF3, BTN_RF4},
    {BTN_LT5, BTN_LT4, BTN_LT1, BTN_LT3, BTN_LT2, BTN_LT6, BTN_RT2, BTN_RT3, BTN_RT1, BTN_RT4, BTN_RT5},
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

constexpr Action fgcActionForButton(uint8_t buttonId)
{
    switch (buttonId) {
        case BTN_LF3: return dpad(GAMEPAD_MASK_LEFT);
        case BTN_LF1: return dpad(GAMEPAD_MASK_RIGHT);
        case BTN_LF2: return dpad(GAMEPAD_MASK_DOWN);
        case BTN_LT1: return dpad(GAMEPAD_MASK_UP);

        case BTN_LF8: return leftAnalog(GAMEPAD_MASK_LEFT);
        case BTN_LF6: return leftAnalog(GAMEPAD_MASK_RIGHT);
        case BTN_LF7: return leftAnalog(GAMEPAD_MASK_DOWN);
        case BTN_LT6: return leftAnalog(GAMEPAD_MASK_UP);

        case BTN_RT3: return rightAnalog(GAMEPAD_MASK_LEFT);
        case BTN_RT5: return rightAnalog(GAMEPAD_MASK_RIGHT);
        case BTN_RT2: return rightAnalog(GAMEPAD_MASK_DOWN);
        case BTN_RT4: return rightAnalog(GAMEPAD_MASK_UP);

        case BTN_RF1: return button(GAMEPAD_MASK_B1);
        case BTN_RF2: return button(GAMEPAD_MASK_B2);
        case BTN_RF5: return button(GAMEPAD_MASK_B3);
        case BTN_RF6: return button(GAMEPAD_MASK_B4);
        case BTN_RF8: return button(GAMEPAD_MASK_L1);
        case BTN_RF7: return button(GAMEPAD_MASK_R1);
        case BTN_RF4: return button(GAMEPAD_MASK_L2);
        case BTN_RF3: return button(GAMEPAD_MASK_R2);

        case BTN_MB6: return button(GAMEPAD_MASK_S1);
        case BTN_MB7: return button(GAMEPAD_MASK_S2);
        case BTN_LT2: return button(GAMEPAD_MASK_L3);
        case BTN_RT1: return button(GAMEPAD_MASK_R3);
        case BTN_MB5: return button(GAMEPAD_MASK_A1);
        case BTN_MB4: return button(GAMEPAD_MASK_A2);

        default: return none();
    }
}

constexpr OutputIcon kMenuButtonIcons[][7] = {
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
    {OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::None, OutputIcon::Home, OutputIcon::XboxBack, OutputIcon::Start},
};

constexpr uint32_t kGlyphOptionsVersion = 8;
constexpr uint8_t kPackedSocdPairSize = 3;
constexpr uint8_t kPackedButtonRemapSize = 2;
constexpr uint8_t kPackedRgbColorSize = 4;

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

    mutableProfilesReady = true;
    for (uint8_t i = 0; i < GlyphProfiles::count(); i++) {
        if (i < kPresetProfileCount) {
            mutableProfiles[i].number = kProfiles[i].number;
            copyName(mutableProfiles[i].name, kProfiles[i].name);
            mutableProfiles[i].layout = kProfiles[i].layout;
            mutableProfiles[i].socdMode = kProfiles[i].socdMode;
            mutableProfiles[i].rgbConfig = kProfiles[i].rgbConfig;
            mutableProfiles[i].backends = kProfiles[i].backends;
        } else {
            mutableProfiles[i].number = i + 1;
            snprintf(mutableProfiles[i].name, sizeof(mutableProfiles[i].name), "Profile %u", i + 1);
            mutableProfiles[i].layout = Layout::Platform;
            mutableProfiles[i].socdMode = SOCD_MODE_NEUTRAL;
            mutableProfiles[i].rgbConfig = i + 1;
            mutableProfiles[i].backends = kModernBackends;
        }
        mutableProfiles[i].socdPairCount = 0;
        mutableProfiles[i].buttonRemapCount = 0;
        mutableProfiles[i].rgbColorCount = 0;
    }

    for (const auto& pair : kPlatformSocdPairs) {
        GlyphProfiles::addSocdPair(1, pair.buttonDir1, pair.buttonDir2, pair.socdType);
        GlyphProfiles::addSocdPair(2, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }
    for (const auto& pair : kUltimateSocdPairs) {
        GlyphProfiles::addSocdPair(3, pair.buttonDir1, pair.buttonDir2, pair.socdType);
        GlyphProfiles::addSocdPair(6, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }
    for (const auto& pair : kFgcSocdPairs) {
        GlyphProfiles::addSocdPair(4, pair.buttonDir1, pair.buttonDir2, pair.socdType);
        GlyphProfiles::addSocdPair(5, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }

    for (const auto& remap : kMeleeRemaps) {
        GlyphProfiles::addButtonRemap(1, remap.physicalButton, remap.activates);
        GlyphProfiles::addButtonRemap(3, remap.physicalButton, remap.activates);
    }
    for (const auto& remap : kBrawlRemaps) {
        GlyphProfiles::addButtonRemap(2, remap.physicalButton, remap.activates);
    }
    for (const auto& remap : kSplitFgcRemaps) {
        GlyphProfiles::addButtonRemap(4, remap.physicalButton, remap.activates);
    }
    for (const auto& remap : kFgcRemaps) {
        GlyphProfiles::addButtonRemap(5, remap.physicalButton, remap.activates);
    }

    for (uint8_t profile = 1; profile <= GlyphProfiles::count(); profile++) {
        const uint8_t rgbConfig = mutableProfiles[profile - 1].rgbConfig;
        switch (rgbConfig) {
            case 1:
            case 2:
            case 3:
            case 7:
            case 8:
                GlyphProfiles::addRgbColor(profile, BTN_LF1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF3, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF4, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LT1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LT2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF3, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF4, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF5, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF6, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF7, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF8, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT3, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT4, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT5, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_MB1, GLYPH_DEFAULT_RGB_COLOR);
                break;
            case 4:
                GlyphProfiles::addRgbColor(profile, BTN_LF1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF3, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF5, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LT1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF2, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF3, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF4, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF5, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF6, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF7, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF8, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF9, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RT1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_MB1, GLYPH_DEFAULT_RGB_COLOR);
                break;
            case 5:
                GlyphProfiles::addRgbColor(profile, BTN_LF8, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF7, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LF6, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_LT6, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF10, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF11, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF12, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF1, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF13, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF14, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF15, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF5, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_RF16, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_MB1, GLYPH_DEFAULT_RGB_COLOR);
                break;
            case 13:
                for (uint8_t button = BTN_LF1; button <= BTN_LF8; button++) GlyphProfiles::addRgbColor(profile, button, GLYPH_DEFAULT_RGB_COLOR);
                for (uint8_t button = BTN_RF1; button <= BTN_RF16; button++) GlyphProfiles::addRgbColor(profile, button, GLYPH_DEFAULT_RGB_COLOR);
                for (uint8_t button = BTN_LT1; button <= BTN_LT6; button++) GlyphProfiles::addRgbColor(profile, button, GLYPH_DEFAULT_RGB_COLOR);
                for (uint8_t button = BTN_RT1; button <= BTN_RT5; button++) GlyphProfiles::addRgbColor(profile, button, GLYPH_DEFAULT_RGB_COLOR);
                GlyphProfiles::addRgbColor(profile, BTN_MB1, GLYPH_DEFAULT_RGB_COLOR);
                break;
            default:
                GlyphProfiles::addRgbColor(profile, BTN_MB1, GLYPH_DEFAULT_RGB_COLOR);
                break;
        }
    }
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

GlyphProfiles::Action actionForButton(Layout layout, uint8_t buttonId)
{
    if (layout == Layout::Fgc || layout == Layout::SplitFgc) {
        return fgcActionForButton(buttonId);
    }

    const Action (&matrix)[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = matrixForLayout(layout);
    for (uint8_t row = 0; row < GlyphProfiles::MatrixRows; row++) {
        for (uint8_t col = 0; col < GlyphProfiles::MatrixCols; col++) {
            if (kGlyphButtonMatrix[row][col] == buttonId) {
                return matrix[row][col];
            }
        }
    }
    return none();
}
}

namespace GlyphProfiles
{
uint8_t count()
{
    return MaxProfiles;
}

const Profile& get(uint8_t profileNumber)
{
    const uint8_t profile = clampProfile(profileNumber);
    return kProfiles[(profile <= kPresetProfileCount ? profile : 1) - 1];
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

        if (source.has_socdPairs && source.socdPairs.size >= kPackedSocdPairSize) {
            clearSocdPairs(i + 1);
            const uint8_t pairCount = (source.socdPairs.size / kPackedSocdPairSize) < MaxSocdPairs ?
                                      (source.socdPairs.size / kPackedSocdPairSize) : MaxSocdPairs;
            for (uint8_t pair = 0; pair < pairCount; pair++) {
                const uint8_t offset = pair * kPackedSocdPairSize;
                addSocdPair(i + 1,
                            source.socdPairs.bytes[offset],
                            source.socdPairs.bytes[offset + 1],
                            source.socdPairs.bytes[offset + 2]);
            }
        }

        if (source.has_buttonRemaps && source.buttonRemaps.size >= kPackedButtonRemapSize) {
            clearButtonRemaps(i + 1);
            const uint8_t remapCount = (source.buttonRemaps.size / kPackedButtonRemapSize) < MaxButtonRemaps ?
                                       (source.buttonRemaps.size / kPackedButtonRemapSize) : MaxButtonRemaps;
            for (uint8_t remap = 0; remap < remapCount; remap++) {
                const uint8_t offset = remap * kPackedButtonRemapSize;
                addButtonRemap(i + 1,
                               source.buttonRemaps.bytes[offset],
                               source.buttonRemaps.bytes[offset + 1]);
            }
        }

        if (source.has_rgbColors && source.rgbColors.size >= kPackedRgbColorSize) {
            clearRgbColors(i + 1);
            const uint8_t colorCount = (source.rgbColors.size / kPackedRgbColorSize) < MaxRgbColors ?
                                       (source.rgbColors.size / kPackedRgbColorSize) : MaxRgbColors;
            for (uint8_t color = 0; color < colorCount; color++) {
                const uint8_t offset = color * kPackedRgbColorSize;
                const uint32_t rgb =
                    (static_cast<uint32_t>(source.rgbColors.bytes[offset + 1]) << 16) |
                    (static_cast<uint32_t>(source.rgbColors.bytes[offset + 2]) << 8) |
                    static_cast<uint32_t>(source.rgbColors.bytes[offset + 3]);
                addRgbColor(i + 1, source.rgbColors.bytes[offset], rgb);
            }
        }
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
        destination.has_socdPairs = true;
        destination.socdPairs.size = 0;
        for (uint8_t pair = 0; pair < source.socdPairCount; pair++) {
            if (static_cast<size_t>(destination.socdPairs.size) + kPackedSocdPairSize > sizeof(destination.socdPairs.bytes)) {
                break;
            }
            uint8_t* destinationPair = &destination.socdPairs.bytes[destination.socdPairs.size];
            destinationPair[0] = source.socdPairs[pair].buttonDir1;
            destinationPair[1] = source.socdPairs[pair].buttonDir2;
            destinationPair[2] = source.socdPairs[pair].socdType;
            destination.socdPairs.size += kPackedSocdPairSize;
        }
        destination.has_buttonRemaps = true;
        destination.buttonRemaps.size = 0;
        for (uint8_t remap = 0; remap < source.buttonRemapCount; remap++) {
            if (static_cast<size_t>(destination.buttonRemaps.size) + kPackedButtonRemapSize > sizeof(destination.buttonRemaps.bytes)) {
                break;
            }
            uint8_t* destinationRemap = &destination.buttonRemaps.bytes[destination.buttonRemaps.size];
            destinationRemap[0] = source.buttonRemaps[remap].physicalButton;
            destinationRemap[1] = source.buttonRemaps[remap].activates;
            destination.buttonRemaps.size += kPackedButtonRemapSize;
        }
        destination.has_rgbColors = true;
        destination.rgbColors.size = 0;
        for (uint8_t color = 0; color < source.rgbColorCount; color++) {
            if (static_cast<size_t>(destination.rgbColors.size) + kPackedRgbColorSize > sizeof(destination.rgbColors.bytes)) {
                break;
            }
            uint8_t* destinationColor = &destination.rgbColors.bytes[destination.rgbColors.size];
            destinationColor[0] = source.rgbColors[color].button;
            destinationColor[1] = static_cast<uint8_t>((source.rgbColors[color].color >> 16) & 0xff);
            destinationColor[2] = static_cast<uint8_t>((source.rgbColors[color].color >> 8) & 0xff);
            destinationColor[3] = static_cast<uint8_t>(source.rgbColors[color].color & 0xff);
            destination.rgbColors.size += kPackedRgbColorSize;
        }
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

uint8_t socdPairCount(uint8_t profileNumber)
{
    return state(profileNumber).socdPairCount;
}

const SocdPair& socdPair(uint8_t profileNumber, uint8_t index)
{
    const ProfileState& profile = state(profileNumber);
    const uint8_t safeIndex = index < profile.socdPairCount ? index : 0;
    return profile.socdPairs[safeIndex];
}

uint8_t buttonRemapCount(uint8_t profileNumber)
{
    return state(profileNumber).buttonRemapCount;
}

const ButtonRemap& buttonRemap(uint8_t profileNumber, uint8_t index)
{
    const ProfileState& profile = state(profileNumber);
    const uint8_t safeIndex = index < profile.buttonRemapCount ? index : 0;
    return profile.buttonRemaps[safeIndex];
}

uint8_t rgbColorCount(uint8_t profileNumber)
{
    return state(profileNumber).rgbColorCount;
}

const RgbColor& rgbColor(uint8_t profileNumber, uint8_t index)
{
    const ProfileState& profile = state(profileNumber);
    const uint8_t safeIndex = index < profile.rgbColorCount ? index : 0;
    return profile.rgbColors[safeIndex];
}

bool rgbColorForButton(uint8_t profileNumber, uint8_t button, uint32_t& color)
{
    const ProfileState& profile = state(profileNumber);
    for (uint8_t index = 0; index < profile.rgbColorCount; index++) {
        if (profile.rgbColors[index].button == button) {
            color = profile.rgbColors[index].color;
            return true;
        }
    }
    color = 0;
    return false;
}

void clearSocdPairs(uint8_t profileNumber)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].socdPairCount = 0;
}

void addSocdPair(uint8_t profileNumber, uint8_t buttonDir1, uint8_t buttonDir2, uint8_t socdType)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    if (profile.socdPairCount >= MaxSocdPairs || buttonDir1 == 0 || buttonDir2 == 0 || socdType == 0) {
        return;
    }
    profile.socdPairs[profile.socdPairCount++] = {buttonDir1, buttonDir2, socdType};
}

void clearButtonRemaps(uint8_t profileNumber)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].buttonRemapCount = 0;
}

void addButtonRemap(uint8_t profileNumber, uint8_t physicalButton, uint8_t activates)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    if (profile.buttonRemapCount >= MaxButtonRemaps || physicalButton == 0) {
        return;
    }
    profile.buttonRemaps[profile.buttonRemapCount++] = {physicalButton, activates};
}

void clearRgbColors(uint8_t profileNumber)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].rgbColorCount = 0;
}

void addRgbColor(uint8_t profileNumber, uint8_t button, uint32_t color)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    if (profile.rgbColorCount >= MaxRgbColors || button == 0) {
        return;
    }
    for (uint8_t index = 0; index < profile.rgbColorCount; index++) {
        if (profile.rgbColors[index].button == button) {
            profile.rgbColors[index].color = color & 0x00ffffff;
            return;
        }
    }
    profile.rgbColors[profile.rgbColorCount++] = {button, color & 0x00ffffff};
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
    if (mask == 0) {
        return "None";
    }
    return "Custom";
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

    const uint8_t profile = clampProfile(profileNumber);
    const uint8_t presetIndex = (profile <= kPresetProfileCount ? profile : 1) - 1;
    return kMenuButtonIcons[presetIndex][menuButtonIndex];
}

Action matrixAction(uint8_t profileNumber, uint8_t row, uint8_t col)
{
    if (row >= MatrixRows || col >= MatrixCols) {
        return {Target::None, 0};
    }

    const Action (&matrix)[MatrixRows][MatrixCols] = matrixForLayout(layout(profileNumber));
    return matrix[row][col];
}

Action buttonAction(uint8_t profileNumber, uint8_t buttonId)
{
    return actionForButton(layout(profileNumber), buttonId);
}

uint8_t matrixButton(uint8_t row, uint8_t col)
{
    if (row >= MatrixRows || col >= MatrixCols) {
        return 0;
    }
    return kGlyphButtonMatrix[row][col];
}
}
