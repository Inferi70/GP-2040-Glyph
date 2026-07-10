#include "glyph/glyph_profiles.h"
#include "gamepad/GamepadState.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

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
using GlyphProfiles::ModProfile;
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
constexpr uint8_t SOCD_DIR1_PRIORITY = 4;
constexpr uint8_t SOCD_DIR2_PRIORITY = 5;
constexpr uint8_t SOCD_1IP = 6;
constexpr uint32_t GLYPH_DEFAULT_RGB_COLOR = 2282478;
constexpr uint8_t MOD_PROFILE_MELEE = 1;
constexpr uint8_t MOD_PROFILE_PROJECT_M = 2;
constexpr uint8_t MOD_PROFILE_ULTIMATE = 3;
constexpr uint8_t MOD_PROFILE_RIVALS = 4;
constexpr uint8_t MOD_PROFILE_RIVALS2 = 5;
constexpr uint8_t MOD_PROFILE_DEFAULT = 6;

constexpr uint8_t defaultModProfileForBehavior(uint32_t behaviorMode)
{
    switch (behaviorMode) {
        case GlyphProfiles::BehaviorMelee: return MOD_PROFILE_MELEE;
        case GlyphProfiles::BehaviorProjectM: return MOD_PROFILE_PROJECT_M;
        case GlyphProfiles::BehaviorUltimate: return MOD_PROFILE_ULTIMATE;
        case GlyphProfiles::BehaviorRivals: return MOD_PROFILE_RIVALS;
        case GlyphProfiles::BehaviorRivals2: return MOD_PROFILE_RIVALS2;
        case GlyphProfiles::BehaviorFgc:
        case GlyphProfiles::BehaviorSmash64:
        case GlyphProfiles::BehaviorUnknown:
        default:
            return MOD_PROFILE_DEFAULT;
    }
}

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
    {1, "Melee",     GlyphProfiles::BehaviorMelee,    Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 1, MOD_PROFILE_MELEE,     kPlatformBackends},
    {2, "Brawl",     GlyphProfiles::BehaviorProjectM, Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 2, MOD_PROFILE_PROJECT_M, kPlatformBackends},
    {3, "Ultimate",  GlyphProfiles::BehaviorUltimate, Layout::Platform, SOCD_MODE_SECOND_INPUT_PRIORITY, 3, MOD_PROFILE_ULTIMATE,  kPlatformBackends},
    {4, "Split FGC", GlyphProfiles::BehaviorFgc,      Layout::SplitFgc, SOCD_MODE_NEUTRAL,               4, MOD_PROFILE_DEFAULT,   kModernBackends},
    {5, "FGC",       GlyphProfiles::BehaviorFgc,      Layout::Fgc,      SOCD_MODE_NEUTRAL,               5, MOD_PROFILE_DEFAULT,   kModernBackends},
    {6, "Smash64",   GlyphProfiles::BehaviorSmash64,  Layout::Platform, SOCD_MODE_NEUTRAL,               6, MOD_PROFILE_DEFAULT,   BackendN64},
};
constexpr uint8_t kPresetProfileCount = sizeof(kProfiles) / sizeof(kProfiles[0]);

constexpr ModProfile kModProfiles[] = {
    {MOD_PROFILE_MELEE,     "Melee",     53, 43, 59, 25, 27, 59, 25, 59, 49, 94, true,  {49, 56, 63, 56}, {42, 41, 39, 29}, {51, 41, 39, 29}, {61, 56, 63, 56}},
    {MOD_PROFILE_PROJECT_M, "Project M", 70, 60, 70, 34, 35, 70, 28, 58, 49, 94, true,  {72, 77, 84, 82}, {61, 55, 50, 36}, {62, 55, 40, 34}, {72, 77, 84, 82}},
    {MOD_PROFILE_ULTIMATE,  "Ultimate",  53, 44, 53, 35, 35, 53, 35, 53, 49, 94, false, {53, 53, 53, 53}, {28, 31, 39, 43}, {28, 31, 49, 43}, {53, 53, 53, 53}},
    {MOD_PROFILE_RIVALS,    "Rivals",    66, 44, 59, 23, 44, 67, 44, 113, 49, 94, false, {51, 49, 52, 49}, {43, 35, 31, 24}, {47, 45, 44, 44}, {57, 63, 74, 90}},
    {MOD_PROFILE_RIVALS2,   "Rivals 2",  92, 72, 92, 42, 76, 71, 51, 84, 49, 94, false, {69, 78, 88, 78}, {59, 57, 55, 41}, {71, 57, 55, 41}, {85, 78, 88, 78}},
    {MOD_PROFILE_DEFAULT,   "Default",   53, 35, 53, 35, 35, 53, 35, 53, 49, 94, false, {53, 53, 53, 53}, {35, 35, 35, 35}, {35, 35, 35, 35}, {53, 53, 53, 53}},
};
constexpr uint8_t kModProfileCount = sizeof(kModProfiles) / sizeof(kModProfiles[0]);

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

constexpr GlyphProfiles::SocdPair kSmash64SocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_NEUTRAL},
    {BTN_LF2, BTN_LT6, SOCD_NEUTRAL},
    {BTN_LF2, BTN_RF4, SOCD_NEUTRAL},
    {BTN_RF7, BTN_RF8, SOCD_NEUTRAL},
    {BTN_RF2, BTN_RF6, SOCD_NEUTRAL},
};

constexpr GlyphProfiles::SocdPair kLegacyFgcSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_NEUTRAL},
    {BTN_LF2, BTN_LT1, SOCD_NEUTRAL},
};

constexpr GlyphProfiles::SocdPair kFgcSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_NEUTRAL},
    {BTN_LF2, BTN_LT1, SOCD_NEUTRAL},
    {BTN_LF8, BTN_LF6, SOCD_NEUTRAL},
    {BTN_LF7, BTN_LT6, SOCD_NEUTRAL},
    {BTN_RT3, BTN_RT5, SOCD_NEUTRAL},
    {BTN_RT2, BTN_RT4, SOCD_NEUTRAL},
};

constexpr GlyphProfiles::SocdPair kSplitFgcSocdPairs[] = {
    {BTN_LF3, BTN_LF1, SOCD_NEUTRAL},
    {BTN_LF2, BTN_RF10, SOCD_NEUTRAL},
};

bool isUpButton(uint8_t button)
{
    switch (button) {
        case BTN_RF4:
        case BTN_RF6:
        case BTN_RF8:
        case BTN_RF10:
        case BTN_RT4:
        case BTN_LT1:
        case BTN_LT6:
            return true;
        default:
            return false;
    }
}

bool isDownButton(uint8_t button)
{
    switch (button) {
        case BTN_LF2:
        case BTN_LF7:
        case BTN_RF2:
        case BTN_RF7:
        case BTN_RT2:
            return true;
        default:
            return false;
    }
}

bool isHorizontalPair(const GlyphProfiles::SocdPair& pair)
{
    return (pair.buttonDir1 == BTN_LF3 && pair.buttonDir2 == BTN_LF1) ||
           (pair.buttonDir1 == BTN_RT3 && pair.buttonDir2 == BTN_RT5);
}

bool isVerticalPair(const GlyphProfiles::SocdPair& pair)
{
    return (isDownButton(pair.buttonDir1) && isUpButton(pair.buttonDir2)) ||
           (isUpButton(pair.buttonDir1) && isDownButton(pair.buttonDir2));
}

const GlyphProfiles::SocdPair* defaultSocdPairsForProfile(const GlyphProfiles::ProfileState& profile, uint8_t& count)
{
    switch (profile.layout) {
        case Layout::Fgc:
            count = sizeof(kFgcSocdPairs) / sizeof(kFgcSocdPairs[0]);
            return kFgcSocdPairs;
        case Layout::SplitFgc:
            count = sizeof(kSplitFgcSocdPairs) / sizeof(kSplitFgcSocdPairs[0]);
            return kSplitFgcSocdPairs;
        case Layout::Platform:
        default:
            break;
    }

    switch (profile.behaviorMode) {
        case GlyphProfiles::BehaviorUltimate:
            count = sizeof(kUltimateSocdPairs) / sizeof(kUltimateSocdPairs[0]);
            return kUltimateSocdPairs;
        case GlyphProfiles::BehaviorSmash64:
            count = sizeof(kSmash64SocdPairs) / sizeof(kSmash64SocdPairs[0]);
            return kSmash64SocdPairs;
        case GlyphProfiles::BehaviorMelee:
        case GlyphProfiles::BehaviorProjectM:
        case GlyphProfiles::BehaviorRivals:
        case GlyphProfiles::BehaviorRivals2:
        default:
            count = sizeof(kPlatformSocdPairs) / sizeof(kPlatformSocdPairs[0]);
            return kPlatformSocdPairs;
    }
}

bool pairMatchesButtons(const GlyphProfiles::SocdPair& pair, uint8_t dir1, uint8_t dir2)
{
    return (pair.buttonDir1 == dir1 && pair.buttonDir2 == dir2) ||
           (pair.buttonDir1 == dir2 && pair.buttonDir2 == dir1);
}

GlyphProfiles::SocdPair canonicalSocdPairForSlot(const GlyphProfiles::ProfileState& profile, uint8_t slot)
{
    switch (profile.layout) {
        case Layout::Fgc:
            switch (slot) {
                case GlyphProfiles::SocdLogicalSlotDpadHorizontal: return {BTN_LF3, BTN_LF1, 0};
                case GlyphProfiles::SocdLogicalSlotDpadVertical: return {BTN_LF2, BTN_LT1, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogHorizontal: return {BTN_LF8, BTN_LF6, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogVertical: return {BTN_LF7, BTN_LT6, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogHorizontal: return {BTN_RT3, BTN_RT5, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogVertical: return {BTN_RT2, BTN_RT4, 0};
                default: return {0, 0, 0};
            }

        case Layout::SplitFgc:
            switch (slot) {
                case GlyphProfiles::SocdLogicalSlotDpadHorizontal: return {BTN_LF3, BTN_LF1, 0};
                case GlyphProfiles::SocdLogicalSlotDpadVertical: return {BTN_LF2, BTN_RF10, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogHorizontal: return {0, 0, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogVertical: return {0, 0, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogHorizontal: return {0, 0, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogVertical: return {0, 0, 0};
                default: return {0, 0, 0};
            }

        case Layout::Platform:
        default:
            if (profile.behaviorMode == GlyphProfiles::BehaviorSmash64) {
                switch (slot) {
                    case GlyphProfiles::SocdLogicalSlotDpadHorizontal:
                    case GlyphProfiles::SocdLogicalSlotLeftAnalogHorizontal:
                        return {BTN_LF3, BTN_LF1, 0};
                    case GlyphProfiles::SocdLogicalSlotDpadVertical:
                        return {BTN_LF2, BTN_LT6, 0};
                    case GlyphProfiles::SocdLogicalSlotLeftAnalogVertical:
                        return {BTN_LF2, BTN_RF4, 0};
                    case GlyphProfiles::SocdLogicalSlotRightAnalogHorizontal:
                        return {BTN_RF7, BTN_RF8, 0};
                    case GlyphProfiles::SocdLogicalSlotRightAnalogVertical:
                        return {BTN_RF2, BTN_RF6, 0};
                    default:
                        return {0, 0, 0};
                }
            }

            switch (slot) {
                case GlyphProfiles::SocdLogicalSlotDpadHorizontal:
                    return {BTN_LF8, BTN_LF6, 0};
                case GlyphProfiles::SocdLogicalSlotDpadVertical:
                    if (profile.behaviorMode == GlyphProfiles::BehaviorProjectM ||
                        profile.behaviorMode == GlyphProfiles::BehaviorUltimate) {
                        return {BTN_RF7, BTN_RF8, 0};
                    }
                    return {BTN_LF7, BTN_LT6, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogHorizontal:
                    return {BTN_LF3, BTN_LF1, 0};
                case GlyphProfiles::SocdLogicalSlotLeftAnalogVertical:
                    return {BTN_LF2, BTN_RF4, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogHorizontal:
                    return {BTN_RT3, BTN_RT5, 0};
                case GlyphProfiles::SocdLogicalSlotRightAnalogVertical:
                    return {BTN_RT2, BTN_RT4, 0};
                default:
                    return {0, 0, 0};
            }
    }
}

bool logicalSocdSlotVisibleForProfile(const GlyphProfiles::ProfileState& profile, uint8_t slot)
{
    if (slot >= GlyphProfiles::LogicalSocdSlotCount) {
        return false;
    }

    if (profile.layout == Layout::SplitFgc) {
        return slot == GlyphProfiles::SocdLogicalSlotDpadHorizontal ||
               slot == GlyphProfiles::SocdLogicalSlotDpadVertical;
    }

    if (profile.behaviorMode == GlyphProfiles::BehaviorSmash64) {
        return slot != GlyphProfiles::SocdLogicalSlotDpadHorizontal;
    }

    const GlyphProfiles::SocdPair canonical = canonicalSocdPairForSlot(profile, slot);
    return canonical.buttonDir1 != 0 && canonical.buttonDir2 != 0;
}

bool pairMatchesLogicalSlot(const GlyphProfiles::ProfileState& profile, const GlyphProfiles::SocdPair& pair, uint8_t slot)
{
    const GlyphProfiles::SocdPair canonical = canonicalSocdPairForSlot(profile, slot);
    if (pairMatchesButtons(pair, canonical.buttonDir1, canonical.buttonDir2)) {
        return true;
    }

    if (profile.layout == Layout::Platform) {
        if (slot == GlyphProfiles::SocdLogicalSlotDpadVertical &&
            (profile.behaviorMode == GlyphProfiles::BehaviorProjectM ||
             profile.behaviorMode == GlyphProfiles::BehaviorUltimate)) {
            return pairMatchesButtons(pair, BTN_LF7, BTN_LT6);
        }
    }

    return false;
}

SOCDMode menuModeForSocdType(uint8_t socdType)
{
    switch (socdType) {
        case SOCD_NEUTRAL:
            return SOCD_MODE_NEUTRAL;
        case SOCD_2IP:
        case SOCD_2IP_NO_REAC:
            return SOCD_MODE_SECOND_INPUT_PRIORITY;
        case SOCD_DIR1_PRIORITY:
        case SOCD_DIR2_PRIORITY:
            return SOCD_MODE_UP_PRIORITY;
        case SOCD_1IP:
            return SOCD_MODE_FIRST_INPUT_PRIORITY;
        default:
            return SOCD_MODE_BYPASS;
    }
}

uint8_t pairTypeForMode(const GlyphProfiles::SocdPair& pair, SOCDMode mode)
{
    switch (mode) {
        case SOCD_MODE_NEUTRAL:
            return SOCD_NEUTRAL;
        case SOCD_MODE_SECOND_INPUT_PRIORITY:
            return SOCD_2IP;
        case SOCD_MODE_FIRST_INPUT_PRIORITY:
            return SOCD_1IP;
        case SOCD_MODE_UP_PRIORITY:
            if (isVerticalPair(pair)) {
                return isUpButton(pair.buttonDir1) ? SOCD_DIR1_PRIORITY : SOCD_DIR2_PRIORITY;
            }
            return SOCD_NEUTRAL;
        case SOCD_MODE_BYPASS:
        default:
            return 0;
    }
}

void rebuildSocdPairsForMode(GlyphProfiles::ProfileState& profile, SOCDMode mode)
{
    if (profile.socdPairCount == 0) {
        uint8_t defaultCount = 0;
        const GlyphProfiles::SocdPair* defaults = defaultSocdPairsForProfile(profile, defaultCount);
        for (uint8_t index = 0; index < defaultCount && index < GlyphProfiles::MaxSocdPairs; index++) {
            profile.socdPairs[index] = defaults[index];
        }
        profile.socdPairCount = defaultCount;
    }

    for (uint8_t index = 0; index < profile.socdPairCount; index++) {
        profile.socdPairs[index].socdType = pairTypeForMode(profile.socdPairs[index], mode);
    }
}

void migrateLegacyFgcSocdPairs(GlyphProfiles::ProfileState& profile)
{
    if (profile.layout == Layout::Fgc &&
        profile.socdPairCount == (sizeof(kLegacyFgcSocdPairs) / sizeof(kLegacyFgcSocdPairs[0])) &&
        pairMatchesButtons(profile.socdPairs[0], BTN_LF3, BTN_LF1) &&
        pairMatchesButtons(profile.socdPairs[1], BTN_LF2, BTN_LT1)) {
        SOCDMode inferredMode = profile.socdMode;
        const SOCDMode pair0Mode = menuModeForSocdType(profile.socdPairs[0].socdType);
        const SOCDMode pair1Mode = menuModeForSocdType(profile.socdPairs[1].socdType);
        if (pair0Mode != SOCD_MODE_BYPASS && pair0Mode == pair1Mode) {
            inferredMode = pair0Mode;
        }

        GlyphProfiles::SocdPair migratedPairs[sizeof(kFgcSocdPairs) / sizeof(kFgcSocdPairs[0])] = {};
        migratedPairs[0] = {BTN_LF3, BTN_LF1, profile.socdPairs[0].socdType};
        migratedPairs[1] = {BTN_LF2, BTN_LT1, profile.socdPairs[1].socdType};
        for (uint8_t index = 2; index < (sizeof(kFgcSocdPairs) / sizeof(kFgcSocdPairs[0])); index++) {
            migratedPairs[index] = kFgcSocdPairs[index];
            migratedPairs[index].socdType = pairTypeForMode(migratedPairs[index], inferredMode);
        }

        profile.socdPairCount = sizeof(migratedPairs) / sizeof(migratedPairs[0]);
        for (uint8_t index = 0; index < profile.socdPairCount; index++) {
            profile.socdPairs[index] = migratedPairs[index];
        }
        return;
    }

    if (profile.layout == Layout::SplitFgc &&
        profile.socdPairCount == (sizeof(kLegacyFgcSocdPairs) / sizeof(kLegacyFgcSocdPairs[0])) &&
        pairMatchesButtons(profile.socdPairs[0], BTN_LF3, BTN_LF1) &&
        pairMatchesButtons(profile.socdPairs[1], BTN_LF2, BTN_LT1)) {
        profile.socdPairs[1].buttonDir1 = BTN_LF2;
        profile.socdPairs[1].buttonDir2 = BTN_RF10;
    }
}

void migrateLegacySmash64SocdPairs(GlyphProfiles::ProfileState& profile)
{
    if (profile.behaviorMode != GlyphProfiles::BehaviorSmash64 || profile.socdPairCount != 4) {
        return;
    }

    const int8_t sharedHorizontalPairIndex =
        pairMatchesButtons(profile.socdPairs[0], BTN_LF3, BTN_LF1) ? 0 :
        pairMatchesButtons(profile.socdPairs[1], BTN_LF3, BTN_LF1) ? 1 :
        pairMatchesButtons(profile.socdPairs[2], BTN_LF3, BTN_LF1) ? 2 :
        pairMatchesButtons(profile.socdPairs[3], BTN_LF3, BTN_LF1) ? 3 : -1;
    const int8_t sharedVerticalPairIndex =
        pairMatchesButtons(profile.socdPairs[0], BTN_LF2, BTN_RF4) ? 0 :
        pairMatchesButtons(profile.socdPairs[1], BTN_LF2, BTN_RF4) ? 1 :
        pairMatchesButtons(profile.socdPairs[2], BTN_LF2, BTN_RF4) ? 2 :
        pairMatchesButtons(profile.socdPairs[3], BTN_LF2, BTN_RF4) ? 3 : -1;
    const int8_t legacyRightHorizontalPairIndex =
        pairMatchesButtons(profile.socdPairs[0], BTN_RT3, BTN_RT5) ? 0 :
        pairMatchesButtons(profile.socdPairs[1], BTN_RT3, BTN_RT5) ? 1 :
        pairMatchesButtons(profile.socdPairs[2], BTN_RT3, BTN_RT5) ? 2 :
        pairMatchesButtons(profile.socdPairs[3], BTN_RT3, BTN_RT5) ? 3 : -1;
    const int8_t legacyRightVerticalPairIndex =
        pairMatchesButtons(profile.socdPairs[0], BTN_RT2, BTN_RT4) ? 0 :
        pairMatchesButtons(profile.socdPairs[1], BTN_RT2, BTN_RT4) ? 1 :
        pairMatchesButtons(profile.socdPairs[2], BTN_RT2, BTN_RT4) ? 2 :
        pairMatchesButtons(profile.socdPairs[3], BTN_RT2, BTN_RT4) ? 3 : -1;

    if (sharedHorizontalPairIndex < 0 || sharedVerticalPairIndex < 0 ||
        legacyRightHorizontalPairIndex < 0 || legacyRightVerticalPairIndex < 0) {
        return;
    }

    const GlyphProfiles::SocdPair migratedPairs[] = {
        {BTN_LF3, BTN_LF1, profile.socdPairs[sharedHorizontalPairIndex].socdType},
        {BTN_LF2, BTN_LT6, profile.socdPairs[sharedVerticalPairIndex].socdType},
        {BTN_LF2, BTN_RF4, profile.socdPairs[sharedVerticalPairIndex].socdType},
        {BTN_RF7, BTN_RF8, profile.socdPairs[legacyRightHorizontalPairIndex].socdType},
        {BTN_RF2, BTN_RF6, profile.socdPairs[legacyRightVerticalPairIndex].socdType},
    };

    profile.socdPairCount = sizeof(migratedPairs) / sizeof(migratedPairs[0]);
    for (uint8_t index = 0; index < profile.socdPairCount; index++) {
        profile.socdPairs[index] = migratedPairs[index];
    }
}

constexpr GlyphProfiles::ButtonRemap kMeleeRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kBrawlRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kUltimateRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kSmash64Remaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kSplitFgcRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kFgcRemaps[] = {
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kRivalsDefaultRemaps[] = {
    {BTN_RF7, BTN_LF7},
    {BTN_RF8, BTN_LT6},
    {BTN_LF6, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LF8, BTN_UNSPECIFIED},
    {BTN_LT3, BTN_UNSPECIFIED},
    {BTN_LT4, BTN_UNSPECIFIED},
    {BTN_LT5, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
    {BTN_RF10, BTN_UNSPECIFIED},
    {BTN_RF11, BTN_UNSPECIFIED},
    {BTN_RF12, BTN_UNSPECIFIED},
    {BTN_RF13, BTN_UNSPECIFIED},
    {BTN_RF14, BTN_UNSPECIFIED},
    {BTN_RF15, BTN_UNSPECIFIED},
    {BTN_RF16, BTN_UNSPECIFIED},
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

constexpr GlyphProfiles::ButtonRemap kRivals2LegacyDefaultRemaps[] = {
    {BTN_RF7, BTN_LF7},
    {BTN_RF8, BTN_LT6},
    {BTN_LF5, BTN_UNSPECIFIED},
    {BTN_LF6, BTN_UNSPECIFIED},
    {BTN_LF7, BTN_UNSPECIFIED},
    {BTN_LF8, BTN_UNSPECIFIED},
    {BTN_LT3, BTN_UNSPECIFIED},
    {BTN_LT4, BTN_UNSPECIFIED},
    {BTN_LT6, BTN_UNSPECIFIED},
    {BTN_RF9, BTN_UNSPECIFIED},
    {BTN_RF10, BTN_UNSPECIFIED},
    {BTN_RF11, BTN_UNSPECIFIED},
    {BTN_RF12, BTN_UNSPECIFIED},
    {BTN_RF13, BTN_UNSPECIFIED},
    {BTN_RF14, BTN_UNSPECIFIED},
    {BTN_RF15, BTN_UNSPECIFIED},
    {BTN_RF16, BTN_UNSPECIFIED},
    {BTN_MB1, BTN_UNSPECIFIED},
    {BTN_MB2, BTN_UNSPECIFIED},
    {BTN_MB3, BTN_UNSPECIFIED},
};

template <size_t N>
bool remapsMatch(const GlyphProfiles::ProfileState& profile, const GlyphProfiles::ButtonRemap (&expected)[N])
{
    if (profile.buttonRemapCount != N) {
        return false;
    }

    for (uint8_t index = 0; index < profile.buttonRemapCount; index++) {
        if (profile.buttonRemaps[index].physicalButton != expected[index].physicalButton ||
            profile.buttonRemaps[index].targetButton != expected[index].targetButton) {
            return false;
        }
    }

    return true;
}

void normalizeLegacyRivals2DefaultRemaps(GlyphProfiles::ProfileState& profile)
{
    if (profile.behaviorMode != GlyphProfiles::BehaviorRivals2 ||
        !remapsMatch(profile, kRivals2LegacyDefaultRemaps)) {
        return;
    }

    profile.buttonRemapCount = sizeof(kRivalsDefaultRemaps) / sizeof(kRivalsDefaultRemaps[0]);
    for (uint8_t index = 0; index < profile.buttonRemapCount; index++) {
        profile.buttonRemaps[index] = kRivalsDefaultRemaps[index];
    }
}

constexpr Action kPlatformMatrix[GlyphProfiles::MatrixRows][GlyphProfiles::MatrixCols] = {
    {button(GAMEPAD_MASK_E1),  button(GAMEPAD_MASK_E2),  button(GAMEPAD_MASK_E3),  button(GAMEPAD_MASK_E4),  button(GAMEPAD_MASK_A1), button(GAMEPAD_MASK_S1), button(GAMEPAD_MASK_S2), button(GAMEPAD_MASK_L2), button(GAMEPAD_MASK_E9), none(), none()},
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

constexpr OutputIcon kDefaultMenuIcons[7] = {
    OutputIcon::None,
    OutputIcon::None,
    OutputIcon::None,
    OutputIcon::None,
    OutputIcon::Home,
    OutputIcon::XboxBack,
    OutputIcon::Start,
};

constexpr uint32_t kGlyphOptionsVersion = 12;
constexpr uint8_t kPackedSocdPairSize = 3;
constexpr uint8_t kPackedButtonRemapSize = 2;
constexpr uint8_t kPackedRgbColorSize = 4;

GlyphProfiles::ProfileState mutableProfiles[GlyphProfiles::MaxProfiles] = {};
GlyphProfiles::ModProfileState mutableModProfiles[GlyphProfiles::MaxModProfiles] = {};
bool mutableProfilesReady = false;
uint8_t configuredProfileCount = kPresetProfileCount;

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

void initializeModProfiles()
{
    for (uint8_t i = 0; i < GlyphProfiles::MaxModProfiles; i++) {
        mutableModProfiles[i].number = i + 1;
        mutableModProfiles[i].enabled = false;
        copyName(mutableModProfiles[i].name, "");
        mutableModProfiles[i].modXHorizontal = 53;
        mutableModProfiles[i].modXVertical = 35;
        mutableModProfiles[i].modXDiagonalX = 53;
        mutableModProfiles[i].modXDiagonalY = 35;
        mutableModProfiles[i].modYHorizontal = 35;
        mutableModProfiles[i].modYVertical = 53;
        mutableModProfiles[i].modYDiagonalX = 35;
        mutableModProfiles[i].modYDiagonalY = 53;
        mutableModProfiles[i].lightShield1 = 49;
        mutableModProfiles[i].lightShield2 = 94;
        mutableModProfiles[i].analogTriggersEnabled = false;
        for (uint8_t slot = 0; slot < 4; slot++) {
            mutableModProfiles[i].modXX[slot] = 53;
            mutableModProfiles[i].modXY[slot] = 35;
            mutableModProfiles[i].modYX[slot] = 35;
            mutableModProfiles[i].modYY[slot] = 53;
        }
    }

    for (uint8_t i = 0; i < kModProfileCount; i++) {
        mutableModProfiles[i].number = kModProfiles[i].number;
        mutableModProfiles[i].enabled = true;
        copyName(mutableModProfiles[i].name, kModProfiles[i].name);
        mutableModProfiles[i].modXHorizontal = kModProfiles[i].modXHorizontal;
        mutableModProfiles[i].modXVertical = kModProfiles[i].modXVertical;
        mutableModProfiles[i].modXDiagonalX = kModProfiles[i].modXDiagonalX;
        mutableModProfiles[i].modXDiagonalY = kModProfiles[i].modXDiagonalY;
        mutableModProfiles[i].modYHorizontal = kModProfiles[i].modYHorizontal;
        mutableModProfiles[i].modYVertical = kModProfiles[i].modYVertical;
        mutableModProfiles[i].modYDiagonalX = kModProfiles[i].modYDiagonalX;
        mutableModProfiles[i].modYDiagonalY = kModProfiles[i].modYDiagonalY;
        mutableModProfiles[i].lightShield1 = kModProfiles[i].lightShield1;
        mutableModProfiles[i].lightShield2 = kModProfiles[i].lightShield2;
        mutableModProfiles[i].analogTriggersEnabled = kModProfiles[i].analogTriggersEnabled;
        for (uint8_t slot = 0; slot < 4; slot++) {
            mutableModProfiles[i].modXX[slot] = kModProfiles[i].modXX[slot];
            mutableModProfiles[i].modXY[slot] = kModProfiles[i].modXY[slot];
            mutableModProfiles[i].modYX[slot] = kModProfiles[i].modYX[slot];
            mutableModProfiles[i].modYY[slot] = kModProfiles[i].modYY[slot];
        }
    }
}

void ensureMutableProfiles()
{
    if (mutableProfilesReady) {
        return;
    }

    mutableProfilesReady = true;
    initializeModProfiles();
    for (uint8_t i = 0; i < GlyphProfiles::count(); i++) {
        if (i < kPresetProfileCount) {
            mutableProfiles[i].number = kProfiles[i].number;
            copyName(mutableProfiles[i].name, kProfiles[i].name);
            mutableProfiles[i].behaviorMode = kProfiles[i].behaviorMode;
            mutableProfiles[i].layout = kProfiles[i].layout;
            mutableProfiles[i].socdMode = kProfiles[i].socdMode;
            mutableProfiles[i].rgbConfig = kProfiles[i].rgbConfig;
            mutableProfiles[i].modProfileId = kProfiles[i].modProfileId;
            mutableProfiles[i].backends = kProfiles[i].backends;
        } else {
            mutableProfiles[i].number = i + 1;
            snprintf(mutableProfiles[i].name, sizeof(mutableProfiles[i].name), "Profile %u", i + 1);
            mutableProfiles[i].behaviorMode = GlyphProfiles::BehaviorMelee;
            mutableProfiles[i].layout = Layout::Platform;
            mutableProfiles[i].socdMode = SOCD_MODE_NEUTRAL;
            mutableProfiles[i].rgbConfig = i + 1;
            mutableProfiles[i].modProfileId = MOD_PROFILE_DEFAULT;
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
    }
    for (const auto& pair : kSmash64SocdPairs) {
        GlyphProfiles::addSocdPair(6, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }
    for (const auto& pair : kSplitFgcSocdPairs) {
        GlyphProfiles::addSocdPair(4, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }
    for (const auto& pair : kFgcSocdPairs) {
        GlyphProfiles::addSocdPair(5, pair.buttonDir1, pair.buttonDir2, pair.socdType);
    }

    for (const auto& remap : kMeleeRemaps) {
        GlyphProfiles::addButtonRemap(1, remap.physicalButton, remap.targetButton);
    }
    for (const auto& remap : kBrawlRemaps) {
        GlyphProfiles::addButtonRemap(2, remap.physicalButton, remap.targetButton);
    }
    for (const auto& remap : kUltimateRemaps) {
        GlyphProfiles::addButtonRemap(3, remap.physicalButton, remap.targetButton);
    }
    for (const auto& remap : kSmash64Remaps) {
        GlyphProfiles::addButtonRemap(6, remap.physicalButton, remap.targetButton);
    }
    for (const auto& remap : kSplitFgcRemaps) {
        GlyphProfiles::addButtonRemap(4, remap.physicalButton, remap.targetButton);
    }
    for (const auto& remap : kFgcRemaps) {
        GlyphProfiles::addButtonRemap(5, remap.physicalButton, remap.targetButton);
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

uint8_t clampModProfile(uint8_t modProfileId)
{
    return (modProfileId >= 1 && modProfileId <= GlyphProfiles::MaxModProfiles) ? modProfileId : MOD_PROFILE_DEFAULT;
}

uint8_t firstVisibleModProfile()
{
    for (uint8_t modProfile = 1; modProfile <= GlyphProfiles::MaxModProfiles; modProfile++) {
        if (mutableModProfiles[modProfile - 1].enabled) {
            return modProfile;
        }
    }
    return MOD_PROFILE_DEFAULT;
}

uint8_t clampModProfileValue(int16_t value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 127) {
        return 127;
    }
    return static_cast<uint8_t>(value);
}

uint8_t deriveAngle(uint8_t horizontal, uint8_t vertical)
{
    if (horizontal == 0 && vertical == 0) {
        return 45;
    }
    float angle = atan2f(static_cast<float>(vertical), static_cast<float>(horizontal)) * (180.0f / 3.14159265f);
    if (angle < 0.0f) {
        angle = 0.0f;
    }
    if (angle > 90.0f) {
        angle = 90.0f;
    }
    return static_cast<uint8_t>(lroundf(angle));
}

void polarToCartesian(uint8_t magnitude, uint8_t angle, uint8_t& x, uint8_t& y)
{
    const float radians = static_cast<float>(angle) * (3.14159265f / 180.0f);
    x = clampModProfileValue(static_cast<int16_t>(lroundf(cosf(radians) * magnitude)));
    y = clampModProfileValue(static_cast<int16_t>(lroundf(sinf(radians) * magnitude)));
}

uint8_t defaultModProfileForLegacyProfile(uint8_t profileNumber)
{
    if (profileNumber >= 1 && profileNumber <= kPresetProfileCount) {
        return kProfiles[profileNumber - 1].modProfileId;
    }
    return MOD_PROFILE_DEFAULT;
}

uint32_t defaultBehaviorForLegacyProfile(uint8_t profileNumber)
{
    const uint8_t profile = (profileNumber >= 1 && profileNumber <= GlyphProfiles::count()) ? profileNumber : 1;
    if (profile <= kPresetProfileCount) {
        return kProfiles[profile - 1].behaviorMode;
    }
    return GlyphProfiles::BehaviorMelee;
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
    if (layout == Layout::Fgc) {
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

bool isDefaultProfileState(uint8_t profileNumber)
{
    const uint8_t profile = clampProfile(profileNumber);
    const ProfileState& current = mutableProfiles[profile - 1];
    const Profile& defaults = get(profile);

    if (strcmp(current.name, defaults.name) != 0) return false;
    if (current.behaviorMode != defaults.behaviorMode) return false;
    if (current.layout != defaults.layout) return false;
    if (current.socdMode != defaults.socdMode) return false;
    if (current.rgbConfig != defaults.rgbConfig) return false;
    if (current.modProfileId != defaults.modProfileId) return false;
    if (current.backends != defaults.backends) return false;
    if (current.socdPairCount != 0) return false;
    if (current.buttonRemapCount != 0) return false;
    if (current.rgbColorCount != 0) return false;

    return true;
}

bool usingDefaultProfileSet()
{
    for (uint8_t profile = 1; profile <= GlyphProfiles::count(); profile++) {
        if (!isDefaultProfileState(profile)) {
            return false;
        }
    }

    return true;
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

uint32_t behaviorMode(uint8_t profileNumber)
{
    return state(profileNumber).behaviorMode;
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

uint8_t modProfile(uint8_t profileNumber)
{
    return state(profileNumber).modProfileId;
}

uint16_t backends(uint8_t profileNumber)
{
    return state(profileNumber).backends;
}

void resetToDefaults()
{
    mutableProfilesReady = false;
    ensureMutableProfiles();
    configuredProfileCount = kPresetProfileCount;
}

void restoreProfileDefaults(uint8_t profileNumber)
{
    ensureMutableProfiles();

    const uint8_t profile = clampProfile(profileNumber);
    const Profile& defaults = get(profile);
    ProfileState& destination = mutableProfiles[profile - 1];

    destination.number = profile;
    copyName(destination.name, defaults.name);
    destination.behaviorMode = defaults.behaviorMode;
    destination.layout = defaults.layout;
    destination.socdMode = defaults.socdMode;
    destination.rgbConfig = defaults.rgbConfig;
    destination.modProfileId = defaults.modProfileId;
    destination.backends = defaults.backends;
    destination.socdPairCount = 0;
    destination.buttonRemapCount = 0;
    destination.rgbColorCount = 0;
}

void loadFromConfig(const GlyphOptions& options)
{
    resetToDefaults();

    const uint8_t modProfileCount = options.modProfiles_count < GlyphProfiles::MaxModProfiles ? options.modProfiles_count : GlyphProfiles::MaxModProfiles;
    for (uint8_t i = 0; i < modProfileCount; i++) {
        const GlyphModProfileEntry& source = options.modProfiles[i];
        ModProfileState& destination = mutableModProfiles[i];
        destination.enabled = source.enabled || (i < kModProfileCount);
        if (source.name[0] != '\0') {
            copyName(destination.name, source.name);
        }
        if (source.modXX_count == 4 && source.modXY_count == 4) {
            for (uint8_t slot = 0; slot < 4; slot++) {
                destination.modXX[slot] = clampModProfileValue(static_cast<int16_t>(source.modXX[slot]));
                destination.modXY[slot] = clampModProfileValue(static_cast<int16_t>(source.modXY[slot]));
            }
        }
        if (source.modYX_count == 4 && source.modYY_count == 4) {
            for (uint8_t slot = 0; slot < 4; slot++) {
                destination.modYX[slot] = clampModProfileValue(static_cast<int16_t>(source.modYX[slot]));
                destination.modYY[slot] = clampModProfileValue(static_cast<int16_t>(source.modYY[slot]));
            }
        }

        if (source.modXX_count == 0 && source.modXY_count == 0 && source.modXAngles_count == 4) {
            for (uint8_t slot = 0; slot < 4; slot++) {
                uint8_t x = 0;
                uint8_t y = 0;
                polarToCartesian(
                    source.has_modXMagnitude ? clampModProfileValue(static_cast<int16_t>(source.modXMagnitude)) : 0,
                    source.modXAngles[slot] <= 90 ? static_cast<uint8_t>(source.modXAngles[slot]) : 45,
                    x,
                    y
                );
                destination.modXX[slot] = x;
                destination.modXY[slot] = y;
            }
        }
        if (source.modYX_count == 0 && source.modYY_count == 0 && source.modYAngles_count == 4) {
            for (uint8_t slot = 0; slot < 4; slot++) {
                uint8_t x = 0;
                uint8_t y = 0;
                polarToCartesian(
                    source.has_modYMagnitude ? clampModProfileValue(static_cast<int16_t>(source.modYMagnitude)) : 0,
                    source.modYAngles[slot] <= 90 ? static_cast<uint8_t>(source.modYAngles[slot]) : 45,
                    x,
                    y
                );
                destination.modYX[slot] = x;
                destination.modYY[slot] = y;
            }
        }

        if (source.modXX_count == 0 && source.modXY_count == 0 && source.has_modXHorizontal && source.has_modXVertical) {
            const uint8_t x = clampModProfileValue(static_cast<int16_t>(source.modXHorizontal));
            const uint8_t y = clampModProfileValue(static_cast<int16_t>(source.modXVertical));
            destination.modXHorizontal = x;
            destination.modXVertical = y;
            for (uint8_t slot = 0; slot < 4; slot++) {
                destination.modXX[slot] = x;
                destination.modXY[slot] = y;
            }
        }
        if (source.modYX_count == 0 && source.modYY_count == 0 && source.has_modYHorizontal && source.has_modYVertical) {
            const uint8_t x = clampModProfileValue(static_cast<int16_t>(source.modYHorizontal));
            const uint8_t y = clampModProfileValue(static_cast<int16_t>(source.modYVertical));
            destination.modYHorizontal = x;
            destination.modYVertical = y;
            for (uint8_t slot = 0; slot < 4; slot++) {
                destination.modYX[slot] = x;
                destination.modYY[slot] = y;
            }
        }

        if (source.has_modXHorizontal) {
            destination.modXHorizontal = clampModProfileValue(static_cast<int16_t>(source.modXHorizontal));
        }
        if (source.has_modXVertical) {
            destination.modXVertical = clampModProfileValue(static_cast<int16_t>(source.modXVertical));
        }
        if (source.has_modXDiagonalX) {
            destination.modXDiagonalX = clampModProfileValue(static_cast<int16_t>(source.modXDiagonalX));
        }
        if (source.has_modXDiagonalY) {
            destination.modXDiagonalY = clampModProfileValue(static_cast<int16_t>(source.modXDiagonalY));
        }
        if (source.has_modYHorizontal) {
            destination.modYHorizontal = clampModProfileValue(static_cast<int16_t>(source.modYHorizontal));
        }
        if (source.has_modYVertical) {
            destination.modYVertical = clampModProfileValue(static_cast<int16_t>(source.modYVertical));
        }
        if (source.has_modYDiagonalX) {
            destination.modYDiagonalX = clampModProfileValue(static_cast<int16_t>(source.modYDiagonalX));
        }
        if (source.has_modYDiagonalY) {
            destination.modYDiagonalY = clampModProfileValue(static_cast<int16_t>(source.modYDiagonalY));
        }
        if (source.has_lightShield1) {
            destination.lightShield1 = clampModProfileValue(static_cast<int16_t>(source.lightShield1));
        }
        if (source.has_lightShield2) {
            destination.lightShield2 = clampModProfileValue(static_cast<int16_t>(source.lightShield2));
        }
        if (source.has_analogTriggersEnabled) {
            destination.analogTriggersEnabled = source.analogTriggersEnabled;
        }
    }

    const uint8_t profileCount = options.profiles_count < count() ? options.profiles_count : count();
    configuredProfileCount = profileCount == 0 ? kPresetProfileCount : profileCount;
    for (uint8_t i = 0; i < profileCount; i++) {
        const GlyphProfileEntry& source = options.profiles[i];
        ProfileState& destination = mutableProfiles[i];

        if (source.name[0] != '\0') {
            copyName(destination.name, source.name);
        }
        destination.behaviorMode = source.has_behaviorMode ? source.behaviorMode : defaultBehaviorForLegacyProfile(i + 1);
        if (validLayout(source.layout)) {
            destination.layout = static_cast<Layout>(source.layout);
        }
        destination.socdMode = source.socdMode;
        destination.rgbConfig = static_cast<uint8_t>(source.rgbConfig);
        destination.modProfileId = source.has_modProfile
            ? clampModProfile(static_cast<uint8_t>(source.modProfile))
            : defaultModProfileForBehavior(destination.behaviorMode);
        if (source.has_modProfile && destination.modProfileId == MOD_PROFILE_DEFAULT && i < kPresetProfileCount) {
            const Profile& preset = kProfiles[i];
            const uint8_t intendedModProfile = defaultModProfileForBehavior(destination.behaviorMode);
            if (intendedModProfile != MOD_PROFILE_DEFAULT &&
                destination.behaviorMode == preset.behaviorMode &&
                destination.layout == preset.layout &&
                destination.socdMode == preset.socdMode &&
                destination.rgbConfig == preset.rgbConfig &&
                destination.backends == preset.backends) {
                destination.modProfileId = intendedModProfile;
            }
        }
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
        migrateLegacyFgcSocdPairs(destination);
        migrateLegacySmash64SocdPairs(destination);

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
        normalizeLegacyRivals2DefaultRemaps(destination);

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
    uint8_t serializedModProfileCount = kModProfileCount;
    for (uint8_t i = kModProfileCount; i < GlyphProfiles::MaxModProfiles; i++) {
        if (mutableModProfiles[i].enabled) {
            serializedModProfileCount = i + 1;
        }
    }

    options.modProfiles_count = serializedModProfileCount;
    for (uint8_t i = 0; i < serializedModProfileCount; i++) {
        GlyphModProfileEntry& destination = options.modProfiles[i];
        const ModProfileState& source = mutableModProfiles[i];
        copyName(destination.name, source.name);
        destination.has_modXHorizontal = true;
        destination.modXHorizontal = source.modXHorizontal;
        destination.has_modXVertical = true;
        destination.modXVertical = source.modXVertical;
        destination.has_modXDiagonalX = true;
        destination.modXDiagonalX = source.modXDiagonalX;
        destination.has_modXDiagonalY = true;
        destination.modXDiagonalY = source.modXDiagonalY;
        destination.has_modYHorizontal = true;
        destination.modYHorizontal = source.modYHorizontal;
        destination.has_modYVertical = true;
        destination.modYVertical = source.modYVertical;
        destination.has_modYDiagonalX = true;
        destination.modYDiagonalX = source.modYDiagonalX;
        destination.has_modYDiagonalY = true;
        destination.modYDiagonalY = source.modYDiagonalY;
        destination.has_lightShield1 = true;
        destination.lightShield1 = source.lightShield1;
        destination.has_lightShield2 = true;
        destination.lightShield2 = source.lightShield2;
        destination.has_analogTriggersEnabled = true;
        destination.analogTriggersEnabled = source.analogTriggersEnabled;
        destination.modXX_count = 4;
        destination.modXY_count = 4;
        destination.modYX_count = 4;
        destination.modYY_count = 4;
        for (uint8_t slot = 0; slot < 4; slot++) {
            destination.modXX[slot] = source.modXX[slot];
            destination.modXY[slot] = source.modXY[slot];
            destination.modYX[slot] = source.modYX[slot];
            destination.modYY[slot] = source.modYY[slot];
        }
        destination.enabled = source.enabled;
    }

    uint8_t serializedProfileCount = usingDefaultProfileSet() ? kPresetProfileCount : 1;
    if (!usingDefaultProfileSet()) {
        for (uint8_t i = 1; i <= count(); i++) {
            if (!isDefaultProfileState(i)) {
                serializedProfileCount = i;
            }
        }
    }

    configuredProfileCount = serializedProfileCount;
    options.profiles_count = serializedProfileCount;
    for (uint8_t i = 0; i < count(); i++) {
        GlyphProfileEntry& destination = options.profiles[i];
        const ProfileState& source = mutableProfiles[i];

        copyName(destination.name, source.name);
        destination.behaviorMode = source.behaviorMode;
        destination.layout = static_cast<uint32_t>(source.layout);
        destination.socdMode = source.socdMode;
        destination.rgbConfig = source.rgbConfig;
        destination.modProfile = source.modProfileId;
        destination.has_modProfile = true;
        destination.behaviorMode = source.behaviorMode;
        destination.has_behaviorMode = true;
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
            destinationRemap[1] = source.buttonRemaps[remap].targetButton;
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

void setBehaviorMode(uint8_t profileNumber, uint32_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].behaviorMode = value;
}

void setLayout(uint8_t profileNumber, Layout value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].layout = value;
}

void setSOCDMode(uint8_t profileNumber, SOCDMode value)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    profile.socdMode = value;
    rebuildSocdPairsForMode(profile, value);
}

void setRgbConfig(uint8_t profileNumber, uint8_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].rgbConfig = value;
}

void setModProfile(uint8_t profileNumber, uint8_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].modProfileId = clampModProfile(value);
}

void setBackends(uint8_t profileNumber, uint16_t value)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].backends = value;
}

void setSocdPairType(uint8_t profileNumber, uint8_t index, uint8_t socdType)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    if (index >= profile.socdPairCount) {
        return;
    }
    profile.socdPairs[index].socdType = socdType;
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

const char* logicalSocdSlotLabel(uint8_t profileNumber, uint8_t slot)
{
    const ProfileState& profile = state(profileNumber);
    if (profile.behaviorMode == GlyphProfiles::BehaviorSmash64 &&
        slot == SocdLogicalSlotLeftAnalogHorizontal) {
        return "LS + D-Pad Left/Right";
    }

    switch (slot) {
        case SocdLogicalSlotDpadHorizontal: return "D-Pad Left/Right";
        case SocdLogicalSlotDpadVertical: return "D-Pad Up/Down";
        case SocdLogicalSlotLeftAnalogHorizontal: return "LS Left/Right";
        case SocdLogicalSlotLeftAnalogVertical: return "LS Up/Down";
        case SocdLogicalSlotRightAnalogHorizontal: return "RS Left/Right";
        case SocdLogicalSlotRightAnalogVertical: return "RS Up/Down";
        default: return "Unknown";
    }
}

bool logicalSocdSlotVisible(uint8_t profileNumber, uint8_t slot)
{
    return logicalSocdSlotVisibleForProfile(state(profileNumber), slot);
}

SOCDMode logicalSocdMode(uint8_t profileNumber, uint8_t slot)
{
    const ProfileState& profile = state(profileNumber);
    for (uint8_t index = 0; index < profile.socdPairCount; index++) {
        if (pairMatchesLogicalSlot(profile, profile.socdPairs[index], slot)) {
            return menuModeForSocdType(profile.socdPairs[index].socdType);
        }
    }

    const GlyphProfiles::SocdPair canonical = canonicalSocdPairForSlot(profile, slot);
    if (canonical.buttonDir1 == 0 || canonical.buttonDir2 == 0) {
        return profile.socdMode;
    }
    return menuModeForSocdType(pairTypeForMode(canonical, profile.socdMode));
}

void setLogicalSocdMode(uint8_t profileNumber, uint8_t slot, SOCDMode mode)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    const GlyphProfiles::SocdPair canonical = canonicalSocdPairForSlot(profile, slot);

    for (uint8_t index = 0; index < profile.socdPairCount; index++) {
        if (!pairMatchesLogicalSlot(profile, profile.socdPairs[index], slot)) {
            continue;
        }
        profile.socdPairs[index].socdType = pairTypeForMode(profile.socdPairs[index], mode);
        return;
    }

    if (canonical.buttonDir1 == 0 || canonical.buttonDir2 == 0) {
        return;
    }
    const SOCDMode fallbackMode = menuModeForSocdType(pairTypeForMode(canonical, profile.socdMode));
    if (mode == fallbackMode || profile.socdPairCount >= MaxSocdPairs) {
        return;
    }

    GlyphProfiles::SocdPair pair = canonical;
    pair.socdType = pairTypeForMode(pair, mode);
    profile.socdPairs[profile.socdPairCount++] = pair;
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
    if (profile.socdPairCount >= MaxSocdPairs || buttonDir1 == 0 || buttonDir2 == 0) {
        return;
    }
    profile.socdPairs[profile.socdPairCount++] = {buttonDir1, buttonDir2, socdType};
}

void clearButtonRemaps(uint8_t profileNumber)
{
    ensureMutableProfiles();
    mutableProfiles[clampProfile(profileNumber) - 1].buttonRemapCount = 0;
}

void addButtonRemap(uint8_t profileNumber, uint8_t physicalButton, uint8_t targetButton)
{
    ensureMutableProfiles();
    ProfileState& profile = mutableProfiles[clampProfile(profileNumber) - 1];
    if (profile.buttonRemapCount >= MaxButtonRemaps || physicalButton == 0) {
        return;
    }
    profile.buttonRemaps[profile.buttonRemapCount++] = {physicalButton, targetButton};
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

bool isVisibleProfile(uint8_t profileNumber)
{
    ensureMutableProfiles();
    const uint8_t profile = clampProfile(profileNumber);

    if (usingDefaultProfileSet()) {
        return profile <= kPresetProfileCount;
    }

    return profile <= configuredProfileCount && !isDefaultProfileState(profile);
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
        case INPUT_MODE_GAMECUBE:
            return (mask & BackendGameCube) != 0;
        default:
            return (mask & BackendDInput) != 0;
    }
}

const char* backendSummary(uint8_t profileNumber)
{
    const uint16_t mask = backends(profileNumber);
    if (mask == BackendGameCube) {
        return "GC";
    }
    if (mask == BackendN64) {
        return "N64";
    }
    if ((mask & (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5 | BackendGameCube)) ==
        (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5 | BackendGameCube)) {
        return "X/XB1/D/SW/PS/GC";
    }
    if ((mask & (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5)) ==
        (BackendXInput | BackendDInput | BackendSwitch | BackendPS4 | BackendPS5)) {
        return "X/XB1/D/SW/PS";
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

uint8_t modProfileCount()
{
    return GlyphProfiles::MaxModProfiles;
}

const ModProfileState& getModProfile(uint8_t modProfileId)
{
    ensureMutableProfiles();
    return mutableModProfiles[clampModProfile(modProfileId) - 1];
}

const char* modProfileName(uint8_t modProfileId)
{
    return getModProfile(modProfileId).name;
}

bool modProfileEditable(uint8_t modProfileId)
{
    return clampModProfile(modProfileId) > kModProfileCount;
}

bool modProfileVisible(uint8_t modProfileId)
{
    return getModProfile(modProfileId).enabled;
}

bool modProfileAnalogTriggersEnabled(uint8_t modProfileId)
{
    return getModProfile(modProfileId).analogTriggersEnabled;
}

bool duplicateModProfile(uint8_t sourceModProfileId, uint8_t& newModProfileId)
{
    ensureMutableProfiles();
    const uint8_t source = clampModProfile(sourceModProfileId);
    for (uint8_t i = kModProfileCount; i < GlyphProfiles::MaxModProfiles; i++) {
        if (mutableModProfiles[i].enabled) {
            continue;
        }

        mutableModProfiles[i] = mutableModProfiles[source - 1];
        mutableModProfiles[i].number = i + 1;
        mutableModProfiles[i].enabled = true;
        snprintf(mutableModProfiles[i].name, sizeof(mutableModProfiles[i].name), "Custom %u", (i + 1) - kModProfileCount);
        newModProfileId = i + 1;
        return true;
    }

    newModProfileId = source;
    return false;
}

void deleteModProfile(uint8_t modProfileId)
{
    ensureMutableProfiles();
    const uint8_t target = clampModProfile(modProfileId);
    if (!modProfileEditable(target)) {
        return;
    }

    mutableModProfiles[target - 1].enabled = false;
    const uint8_t replacement = firstVisibleModProfile();

    for (uint8_t profile = 1; profile <= count(); profile++) {
        if (mutableProfiles[profile - 1].modProfileId == target) {
            mutableProfiles[profile - 1].modProfileId = replacement;
        }
    }
}

void adjustModProfileCoordinate(uint8_t modProfileId, bool modX, uint8_t slot, bool yAxis, int8_t delta)
{
    ensureMutableProfiles();
    ModProfileState& profile = mutableModProfiles[clampModProfile(modProfileId) - 1];
    if (!profile.enabled || slot >= 6) {
        return;
    }

    if (modX) {
        if (slot == 4) {
            if (yAxis) {
                profile.modXVertical = clampModProfileValue(static_cast<int16_t>(profile.modXVertical) + delta);
            } else {
                profile.modXHorizontal = clampModProfileValue(static_cast<int16_t>(profile.modXHorizontal) + delta);
            }
        } else if (slot == 5) {
            if (yAxis) {
                profile.modXDiagonalY = clampModProfileValue(static_cast<int16_t>(profile.modXDiagonalY) + delta);
            } else {
                profile.modXDiagonalX = clampModProfileValue(static_cast<int16_t>(profile.modXDiagonalX) + delta);
            }
        } else if (yAxis) {
            profile.modXY[slot] = clampModProfileValue(static_cast<int16_t>(profile.modXY[slot]) + delta);
        } else {
            profile.modXX[slot] = clampModProfileValue(static_cast<int16_t>(profile.modXX[slot]) + delta);
        }
    } else {
        if (slot == 4) {
            if (yAxis) {
                profile.modYVertical = clampModProfileValue(static_cast<int16_t>(profile.modYVertical) + delta);
            } else {
                profile.modYHorizontal = clampModProfileValue(static_cast<int16_t>(profile.modYHorizontal) + delta);
            }
        } else if (slot == 5) {
            if (yAxis) {
                profile.modYDiagonalY = clampModProfileValue(static_cast<int16_t>(profile.modYDiagonalY) + delta);
            } else {
                profile.modYDiagonalX = clampModProfileValue(static_cast<int16_t>(profile.modYDiagonalX) + delta);
            }
        } else if (yAxis) {
            profile.modYY[slot] = clampModProfileValue(static_cast<int16_t>(profile.modYY[slot]) + delta);
        } else {
            profile.modYX[slot] = clampModProfileValue(static_cast<int16_t>(profile.modYX[slot]) + delta);
        }
    }
}

void adjustModProfileLightShield(uint8_t modProfileId, bool secondary, int8_t delta)
{
    ensureMutableProfiles();
    ModProfileState& profile = mutableModProfiles[clampModProfile(modProfileId) - 1];
    if (!profile.enabled) {
        return;
    }

    if (secondary) {
        profile.lightShield2 = clampModProfileValue(static_cast<int16_t>(profile.lightShield2) + delta);
    } else {
        profile.lightShield1 = clampModProfileValue(static_cast<int16_t>(profile.lightShield1) + delta);
    }
}

void setModProfileAnalogTriggersEnabled(uint8_t modProfileId, bool value)
{
    ensureMutableProfiles();
    ModProfileState& profile = mutableModProfiles[clampModProfile(modProfileId) - 1];
    if (!profile.enabled) {
        return;
    }
    profile.analogTriggersEnabled = value;
}

void restoreModProfileDefaults(uint8_t modProfileId)
{
    ensureMutableProfiles();
    const uint8_t target = clampModProfile(modProfileId);
    if (target > kModProfileCount) {
        return;
    }

    const ModProfile& defaults = kModProfiles[target - 1];
    ModProfileState& profile = mutableModProfiles[target - 1];
    profile.number = defaults.number;
    copyName(profile.name, defaults.name);
    profile.modXHorizontal = defaults.modXHorizontal;
    profile.modXVertical = defaults.modXVertical;
    profile.modXDiagonalX = defaults.modXDiagonalX;
    profile.modXDiagonalY = defaults.modXDiagonalY;
    profile.modYHorizontal = defaults.modYHorizontal;
    profile.modYVertical = defaults.modYVertical;
    profile.modYDiagonalX = defaults.modYDiagonalX;
    profile.modYDiagonalY = defaults.modYDiagonalY;
    profile.lightShield1 = defaults.lightShield1;
    profile.lightShield2 = defaults.lightShield2;
    profile.analogTriggersEnabled = defaults.analogTriggersEnabled;
    for (uint8_t slot = 0; slot < 4; slot++) {
        profile.modXX[slot] = defaults.modXX[slot];
        profile.modXY[slot] = defaults.modXY[slot];
        profile.modYX[slot] = defaults.modYX[slot];
        profile.modYY[slot] = defaults.modYY[slot];
    }
    profile.enabled = true;
}

uint8_t defaultModProfileForLegacyMode(uint32_t mode)
{
    return defaultModProfileForBehavior(mode);
}

OutputIcon menuIcon(uint8_t profileNumber, uint8_t menuButtonIndex)
{
    if (menuButtonIndex >= std::size(kDefaultMenuIcons)) {
        return OutputIcon::None;
    }

    if (menuButtonIndex == 1 && buttonAvailable(profileNumber, BTN_MB2)) {
        return OutputIcon::Turbo;
    }
    return kDefaultMenuIcons[menuButtonIndex];
}

Action buttonAction(uint8_t profileNumber, uint8_t buttonId)
{
    return actionForButton(layout(profileNumber), buttonId);
}

bool buttonAvailable(uint8_t profileNumber, uint8_t buttonId)
{
    if (buttonId == 0) {
        return false;
    }

    for (uint8_t index = 0; index < buttonRemapCount(profileNumber); index++) {
        const ButtonRemap& remap = buttonRemap(profileNumber, index);
        if (remap.physicalButton == buttonId) {
            return remap.targetButton == BTN_UNSPECIFIED;
        }
    }

    const Action action = buttonAction(profileNumber, buttonId);
    return action.target == Target::None;
}

uint8_t matrixButton(uint8_t row, uint8_t col)
{
    if (row >= MatrixRows || col >= MatrixCols) {
        return 0;
    }
    return kGlyphButtonMatrix[row][col];
}
}
