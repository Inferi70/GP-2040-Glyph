#include "glyph/glyph_webserial.h"

#include "eventmanager.h"
#include "events/GPStorageSaveEvent.h"
#include "gamepad/GamepadState.h"
#include "glyph/glyph_profiles.h"
#include "storagemanager.h"
#include "tusb.h"

#include <stdint.h>
#include <string.h>
#include <vector>

namespace
{
enum Command : uint8_t {
    CMD_UNSPECIFIED = 0,
    CMD_GET_DEVICE_INFO = 1,
    CMD_SET_DEVICE_INFO = 2,
    CMD_GET_CONFIG = 3,
    CMD_SET_CONFIG = 4,
    CMD_ERROR = 5,
    CMD_SUCCESS = 6,
    CMD_REBOOT_FIRMWARE = 7,
    CMD_REBOOT_BOOTLOADER = 8,
};

enum HayboxBackend : uint32_t {
    COMMS_BACKEND_DINPUT = 1,
    COMMS_BACKEND_XINPUT = 2,
    COMMS_BACKEND_GAMECUBE = 3,
    COMMS_BACKEND_N64 = 4,
    COMMS_BACKEND_NINTENDO_SWITCH = 7,
    COMMS_BACKEND_PASSTHROUGH_PS4 = 9,
    COMMS_BACKEND_PASSTHROUGH_PS5 = 10,
};

enum HayboxMode : uint32_t {
    MODE_MELEE = 1,
    MODE_PROJECT_M = 2,
    MODE_ULTIMATE = 3,
    MODE_FGC = 4,
    MODE_RIVALS_OF_AETHER = 5,
    MODE_KEYBOARD = 6,
    MODE_CUSTOM = 7,
    MODE_64 = 8,
    MODE_RIVALS2 = 9,
};

enum HayboxLayout : uint32_t {
    LAYOUT_PLATE_EVERYTHING = 1,
    LAYOUT_PLATE_FGC = 2,
    LAYOUT_PLATE_SPLIT_FGC = 3,
    LAYOUT_PLATE_PLATFORM_FIGHTER = 4,
};

enum HayboxSocd : uint32_t {
    SOCD_NEUTRAL = 1,
    SOCD_2IP = 2,
    SOCD_DIR1_PRIORITY = 4,
    SOCD_DIR2_PRIORITY = 5,
};

static_assert(MODE_MELEE == ::MODE_MELEE);
static_assert(MODE_PROJECT_M == ::MODE_PROJECT_M);
static_assert(MODE_ULTIMATE == ::MODE_ULTIMATE);
static_assert(MODE_FGC == ::MODE_FGC);
static_assert(MODE_RIVALS_OF_AETHER == ::MODE_RIVALS_OF_AETHER);
static_assert(MODE_KEYBOARD == ::MODE_KEYBOARD);
static_assert(MODE_CUSTOM == ::MODE_CUSTOM);
static_assert(MODE_64 == ::MODE_64);
static_assert(MODE_RIVALS2 == ::MODE_RIVALS2);
static_assert(LAYOUT_PLATE_EVERYTHING == ::LAYOUT_PLATE_EVERYTHING);
static_assert(LAYOUT_PLATE_FGC == ::LAYOUT_PLATE_FGC);
static_assert(LAYOUT_PLATE_SPLIT_FGC == ::LAYOUT_PLATE_SPLIT_FGC);
static_assert(LAYOUT_PLATE_PLATFORM_FIGHTER == ::LAYOUT_PLATE_PLATFORM_FIGHTER);
static_assert(COMMS_BACKEND_DINPUT == ::COMMS_BACKEND_DINPUT);
static_assert(COMMS_BACKEND_XINPUT == ::COMMS_BACKEND_XINPUT);
static_assert(COMMS_BACKEND_GAMECUBE == ::COMMS_BACKEND_GAMECUBE);
static_assert(COMMS_BACKEND_N64 == ::COMMS_BACKEND_N64);
static_assert(COMMS_BACKEND_NINTENDO_SWITCH == ::COMMS_BACKEND_NINTENDO_SWITCH);
static_assert(COMMS_BACKEND_PASSTHROUGH_PS4 == ::COMMS_BACKEND_PASSTHROUGH_PS4);
static_assert(COMMS_BACKEND_PASSTHROUGH_PS5 == ::COMMS_BACKEND_PASSTHROUGH_PS5);

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

constexpr size_t kPacketMax = 16384;
constexpr uint8_t kMaxWebRgbConfigs = 30;
uint8_t encodedPacket[kPacketMax] = {};
uint8_t decodedPacket[kPacketMax] = {};
size_t encodedLength = 0;

struct WebRgbConfig {
    uint8_t colorCount;
    GlyphProfiles::RgbColor colors[GlyphProfiles::MaxRgbColors];
};

WebRgbConfig parsedRgbConfigs[kMaxWebRgbConfigs] = {};

enum class ImportedTargetKind : uint8_t {
    Unresolved,
    None,
    Action,
    ModX,
    ModY,
};

struct ImportedTarget {
    ImportedTargetKind kind;
    GlyphProfiles::Action action;
};

constexpr GlyphProfiles::ButtonRemap kIncomingRivalsDefaultRemaps[] = {
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

constexpr GlyphProfiles::ButtonRemap kIncomingRivals2DefaultRemaps[] = {
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

constexpr GlyphProfiles::Action importedNoneAction()
{
    return {GlyphProfiles::Target::None, 0};
}

constexpr ImportedTarget unresolvedImportedTarget()
{
    return {ImportedTargetKind::Unresolved, importedNoneAction()};
}

constexpr ImportedTarget noneImportedTarget()
{
    return {ImportedTargetKind::None, importedNoneAction()};
}

constexpr ImportedTarget importedButton(uint32_t mask)
{
    return {ImportedTargetKind::Action, {GlyphProfiles::Target::Button, mask}};
}

constexpr ImportedTarget importedDpad(uint32_t mask)
{
    return {ImportedTargetKind::Action, {GlyphProfiles::Target::Dpad, mask}};
}

constexpr ImportedTarget importedLeftAnalog(uint32_t mask)
{
    return {ImportedTargetKind::Action, {GlyphProfiles::Target::LeftAnalog, mask}};
}

constexpr ImportedTarget importedRightAnalog(uint32_t mask)
{
    return {ImportedTargetKind::Action, {GlyphProfiles::Target::RightAnalog, mask}};
}

constexpr ImportedTarget importedAux(uint32_t mask)
{
    return {ImportedTargetKind::Action, {GlyphProfiles::Target::Aux, mask}};
}

constexpr ImportedTarget importedModX()
{
    return {ImportedTargetKind::ModX, importedNoneAction()};
}

constexpr ImportedTarget importedModY()
{
    return {ImportedTargetKind::ModY, importedNoneAction()};
}

GlyphProfiles::Layout layoutFromHaybox(uint32_t mode, uint32_t layout);

bool actionsEqual(const GlyphProfiles::Action& lhs, const GlyphProfiles::Action& rhs)
{
    return lhs.target == rhs.target && lhs.mask == rhs.mask;
}

bool remapsMatch(
    const GlyphProfiles::ButtonRemap* lhs,
    uint8_t lhsCount,
    const GlyphProfiles::ButtonRemap* rhs,
    size_t rhsCount
)
{
    if (lhsCount != rhsCount) {
        return false;
    }

    for (uint8_t index = 0; index < lhsCount; index++) {
        if (lhs[index].physicalButton != rhs[index].physicalButton ||
            lhs[index].targetButton != rhs[index].targetButton) {
            return false;
        }
    }

    return true;
}

void normalizeImportedRivals2Defaults(
    uint32_t mode,
    GlyphProfiles::ButtonRemap* buttonRemaps,
    uint8_t& buttonRemapCount
)
{
    if (mode != MODE_RIVALS2) {
        return;
    }

    if (!remapsMatch(
            buttonRemaps,
            buttonRemapCount,
            kIncomingRivals2DefaultRemaps,
            sizeof(kIncomingRivals2DefaultRemaps) / sizeof(kIncomingRivals2DefaultRemaps[0])
        )) {
        return;
    }

    buttonRemapCount = sizeof(kIncomingRivalsDefaultRemaps) / sizeof(kIncomingRivalsDefaultRemaps[0]);
    for (uint8_t index = 0; index < buttonRemapCount; index++) {
        buttonRemaps[index] = kIncomingRivalsDefaultRemaps[index];
    }
}

ImportedTarget incomingLegacyPlatformTarget(uint32_t mode, uint8_t buttonId)
{
    switch (buttonId) {
        case BTN_UNSPECIFIED: return noneImportedTarget();
        case BTN_LT1: return importedModX();
        case BTN_LT2: return importedModY();
        case BTN_RT1: return importedButton(GAMEPAD_MASK_B1);
        case BTN_RF1: return importedButton(GAMEPAD_MASK_B2);
        case BTN_RF2: return importedButton(GAMEPAD_MASK_B3);
        case BTN_RF6: return importedButton(GAMEPAD_MASK_B4);
        case BTN_RF3: return importedButton(GAMEPAD_MASK_R1);
        case BTN_LF4: return importedButton(GAMEPAD_MASK_L2);
        case BTN_RF5: return importedButton(GAMEPAD_MASK_R2);
        case BTN_MB6: return importedButton(GAMEPAD_MASK_S1);
        case BTN_MB7: return importedButton(GAMEPAD_MASK_S2);
        case BTN_MB4: return importedButton(GAMEPAD_MASK_A2);
        case BTN_MB5: return importedButton(GAMEPAD_MASK_A1);
        case BTN_RF7: return importedButton(GAMEPAD_MASK_L3);
        case BTN_RF8: return importedButton(GAMEPAD_MASK_R3);
        case BTN_LF8: return importedDpad(GAMEPAD_MASK_LEFT);
        case BTN_LF6: return importedDpad(GAMEPAD_MASK_RIGHT);
        case BTN_LF7: return importedDpad(GAMEPAD_MASK_DOWN);
        case BTN_LT6: return importedDpad(GAMEPAD_MASK_UP);
        case BTN_LF3: return importedLeftAnalog(GAMEPAD_MASK_LEFT);
        case BTN_LF1: return importedLeftAnalog(GAMEPAD_MASK_RIGHT);
        case BTN_LF2: return importedLeftAnalog(GAMEPAD_MASK_DOWN);
        case BTN_RF4: return importedLeftAnalog(GAMEPAD_MASK_UP);
        case BTN_RT3: return importedRightAnalog(GAMEPAD_MASK_LEFT);
        case BTN_RT5: return importedRightAnalog(GAMEPAD_MASK_RIGHT);
        case BTN_RT2: return importedRightAnalog(GAMEPAD_MASK_DOWN);
        case BTN_RT4: return importedRightAnalog(GAMEPAD_MASK_UP);
        case BTN_RF9:
            return mode == MODE_RIVALS_OF_AETHER ? importedButton(GAMEPAD_MASK_L1) : unresolvedImportedTarget();
        case BTN_LT5:
            return mode == MODE_RIVALS2 ? importedButton(GAMEPAD_MASK_L1) : unresolvedImportedTarget();
        default:
            return unresolvedImportedTarget();
    }
}

uint8_t localLegacyRivalsButton(uint32_t behaviorMode, const GlyphProfiles::Action& action)
{
    switch (action.target) {
        case GlyphProfiles::Target::Button:
            switch (action.mask) {
                case GAMEPAD_MASK_B1: return BTN_RT1;
                case GAMEPAD_MASK_B2: return BTN_RF1;
                case GAMEPAD_MASK_B3: return BTN_RF2;
                case GAMEPAD_MASK_B4: return BTN_RF6;
                case GAMEPAD_MASK_L1: return BTN_RF9;
                case GAMEPAD_MASK_R1: return BTN_RF3;
                case GAMEPAD_MASK_L2: return BTN_LF4;
                case GAMEPAD_MASK_R2: return BTN_RF5;
                case GAMEPAD_MASK_S1: return BTN_MB6;
                case GAMEPAD_MASK_S2: return BTN_MB7;
                case GAMEPAD_MASK_A1: return BTN_MB5;
                case GAMEPAD_MASK_L3: return BTN_RF7;
                case GAMEPAD_MASK_R3: return BTN_RF8;
                case GAMEPAD_MASK_A2: return BTN_MB4;
                default: return BTN_UNSPECIFIED;
            }
        case GlyphProfiles::Target::Aux:
            return BTN_UNSPECIFIED;
        case GlyphProfiles::Target::Dpad:
            switch (action.mask) {
                case GAMEPAD_MASK_LEFT: return BTN_LF8;
                case GAMEPAD_MASK_RIGHT: return BTN_LF6;
                case GAMEPAD_MASK_DOWN: return BTN_LF7;
                case GAMEPAD_MASK_UP: return BTN_LT6;
                default: return BTN_UNSPECIFIED;
            }
        case GlyphProfiles::Target::LeftAnalog:
            switch (action.mask) {
                case GAMEPAD_MASK_LEFT: return BTN_LF3;
                case GAMEPAD_MASK_RIGHT: return BTN_LF1;
                case GAMEPAD_MASK_DOWN: return BTN_LF2;
                case GAMEPAD_MASK_UP: return BTN_RF4;
                default: return BTN_UNSPECIFIED;
            }
        case GlyphProfiles::Target::RightAnalog:
            switch (action.mask) {
                case GAMEPAD_MASK_LEFT: return BTN_RT3;
                case GAMEPAD_MASK_RIGHT: return BTN_RT5;
                case GAMEPAD_MASK_DOWN: return BTN_RT2;
                case GAMEPAD_MASK_UP: return BTN_RT4;
                default: return BTN_UNSPECIFIED;
            }
        case GlyphProfiles::Target::None:
        default:
            return BTN_UNSPECIFIED;
    }
}

uint8_t localPhysicalButtonForImportedTarget(uint8_t profileNumber, const ImportedTarget& target)
{
    if (target.kind == ImportedTargetKind::None) {
        return BTN_UNSPECIFIED;
    }

    if (target.kind == ImportedTargetKind::ModX) {
        return BTN_LT1;
    }

    if (target.kind == ImportedTargetKind::ModY) {
        return BTN_LT2;
    }

    if (target.kind != ImportedTargetKind::Action) {
        return BTN_UNSPECIFIED;
    }

    const uint32_t behaviorMode = GlyphProfiles::behaviorMode(profileNumber);
    if (GlyphProfiles::layout(profileNumber) == GlyphProfiles::Layout::Platform &&
        (behaviorMode == GlyphProfiles::BehaviorRivals || behaviorMode == GlyphProfiles::BehaviorRivals2)) {
        return localLegacyRivalsButton(behaviorMode, target.action);
    }

    for (uint8_t row = 0; row < GlyphProfiles::MatrixRows; row++) {
        for (uint8_t col = 0; col < GlyphProfiles::MatrixCols; col++) {
            const uint8_t buttonId = GlyphProfiles::matrixButton(row, col);
            if (buttonId == BTN_UNSPECIFIED) {
                continue;
            }
            if (actionsEqual(GlyphProfiles::buttonAction(profileNumber, buttonId), target.action)) {
                return buttonId;
            }
        }
    }

    return BTN_UNSPECIFIED;
}

uint8_t translateImportedRemapTarget(uint32_t mode, uint32_t layout, uint8_t profileNumber, uint8_t targetButton)
{
    if (targetButton == BTN_UNSPECIFIED) {
        return BTN_UNSPECIFIED;
    }

    const GlyphProfiles::Layout importedLayout = layoutFromHaybox(mode, layout);
    ImportedTarget target = unresolvedImportedTarget();
    if (importedLayout == GlyphProfiles::Layout::Platform &&
        (mode == MODE_RIVALS_OF_AETHER || mode == MODE_RIVALS2)) {
        target = incomingLegacyPlatformTarget(mode, targetButton);
    }

    if (target.kind == ImportedTargetKind::Unresolved) {
        return targetButton;
    }

    const uint8_t translated = localPhysicalButtonForImportedTarget(profileNumber, target);
    return translated != BTN_UNSPECIFIED ? translated : targetButton;
}

uint8_t translateExportedRemapTarget(uint32_t mode, uint8_t targetButton)
{
    if (mode == MODE_RIVALS2 && targetButton == BTN_RF9) {
        return BTN_LT5;
    }

    return targetButton;
}

void writeVarint(std::vector<uint8_t>& output, uint32_t value)
{
    while (value > 0x7f) {
        output.push_back(static_cast<uint8_t>((value & 0x7f) | 0x80));
        value >>= 7;
    }
    output.push_back(static_cast<uint8_t>(value));
}

void writeTag(std::vector<uint8_t>& output, uint32_t field, uint8_t wireType)
{
    writeVarint(output, (field << 3) | wireType);
}

void writeUInt(std::vector<uint8_t>& output, uint32_t field, uint32_t value)
{
    if (value == 0) {
        return;
    }
    writeTag(output, field, 0);
    writeVarint(output, value);
}

void writeUIntAllowZero(std::vector<uint8_t>& output, uint32_t field, uint32_t value)
{
    writeTag(output, field, 0);
    writeVarint(output, value);
}

void writeString(std::vector<uint8_t>& output, uint32_t field, const char* value)
{
    if (value == nullptr || value[0] == '\0') {
        return;
    }
    const size_t length = strlen(value);
    writeTag(output, field, 2);
    writeVarint(output, static_cast<uint32_t>(length));
    output.insert(output.end(), value, value + length);
}

void writeMessage(std::vector<uint8_t>& output, uint32_t field, const std::vector<uint8_t>& message)
{
    writeTag(output, field, 2);
    writeVarint(output, static_cast<uint32_t>(message.size()));
    output.insert(output.end(), message.begin(), message.end());
}

bool readVarint(const uint8_t*& cursor, const uint8_t* end, uint32_t& value)
{
    value = 0;
    uint8_t shift = 0;
    while (cursor < end && shift < 32) {
        const uint8_t byte = *cursor++;
        value |= static_cast<uint32_t>(byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            return true;
        }
        shift += 7;
    }
    return false;
}

bool skipField(const uint8_t*& cursor, const uint8_t* end, uint8_t wireType)
{
    uint32_t value = 0;
    switch (wireType) {
        case 0:
            return readVarint(cursor, end, value);
        case 1:
            if (end - cursor < 8) return false;
            cursor += 8;
            return true;
        case 2:
            if (!readVarint(cursor, end, value) || static_cast<size_t>(end - cursor) < value) return false;
            cursor += value;
            return true;
        case 5:
            if (end - cursor < 4) return false;
            cursor += 4;
            return true;
        default:
            return false;
    }
}

uint16_t backendMaskFromHaybox(uint32_t backend)
{
    switch (backend) {
        case COMMS_BACKEND_DINPUT: return GlyphProfiles::BackendDInput;
        case COMMS_BACKEND_XINPUT: return GlyphProfiles::BackendXInput;
        case COMMS_BACKEND_GAMECUBE: return GlyphProfiles::BackendGameCube;
        case COMMS_BACKEND_N64: return GlyphProfiles::BackendN64;
        case COMMS_BACKEND_NINTENDO_SWITCH: return GlyphProfiles::BackendSwitch;
        case COMMS_BACKEND_PASSTHROUGH_PS4: return GlyphProfiles::BackendPS4;
        case COMMS_BACKEND_PASSTHROUGH_PS5: return GlyphProfiles::BackendPS5;
        default: return 0;
    }
}

uint32_t hayboxBackendFromMask(uint16_t backend)
{
    switch (backend) {
        case GlyphProfiles::BackendDInput: return COMMS_BACKEND_DINPUT;
        case GlyphProfiles::BackendXInput: return COMMS_BACKEND_XINPUT;
        case GlyphProfiles::BackendGameCube: return COMMS_BACKEND_GAMECUBE;
        case GlyphProfiles::BackendN64: return COMMS_BACKEND_N64;
        case GlyphProfiles::BackendSwitch: return COMMS_BACKEND_NINTENDO_SWITCH;
        case GlyphProfiles::BackendPS4: return COMMS_BACKEND_PASSTHROUGH_PS4;
        case GlyphProfiles::BackendPS5: return COMMS_BACKEND_PASSTHROUGH_PS5;
        default: return 0;
    }
}

GlyphProfiles::Layout layoutFromHaybox(uint32_t mode, uint32_t layout)
{
    if (layout == LAYOUT_PLATE_FGC || mode == MODE_FGC) {
        return GlyphProfiles::Layout::Fgc;
    }
    if (layout == LAYOUT_PLATE_SPLIT_FGC) {
        return GlyphProfiles::Layout::SplitFgc;
    }
    return GlyphProfiles::Layout::Platform;
}

uint32_t behaviorFromHaybox(uint32_t mode, GlyphProfiles::Layout layout, uint32_t fallback)
{
    switch (mode) {
        case MODE_MELEE: return GlyphProfiles::BehaviorMelee;
        case MODE_PROJECT_M: return GlyphProfiles::BehaviorProjectM;
        case MODE_ULTIMATE: return GlyphProfiles::BehaviorUltimate;
        case MODE_FGC: return GlyphProfiles::BehaviorFgc;
        case MODE_RIVALS_OF_AETHER: return GlyphProfiles::BehaviorRivals;
        case MODE_RIVALS2: return GlyphProfiles::BehaviorRivals2;
        case MODE_64: return GlyphProfiles::BehaviorSmash64;
        case MODE_KEYBOARD:
            return layout == GlyphProfiles::Layout::Fgc || layout == GlyphProfiles::Layout::SplitFgc ?
                GlyphProfiles::BehaviorFgc : fallback;
        default:
            return fallback;
    }
}

uint32_t modeFromBehavior(uint32_t behaviorMode, GlyphProfiles::Layout layout, uint8_t modProfileId)
{
    switch (behaviorMode) {
        case GlyphProfiles::BehaviorMelee: return MODE_MELEE;
        case GlyphProfiles::BehaviorProjectM: return MODE_PROJECT_M;
        case GlyphProfiles::BehaviorUltimate: return MODE_ULTIMATE;
        case GlyphProfiles::BehaviorFgc: return MODE_FGC;
        case GlyphProfiles::BehaviorRivals: return MODE_RIVALS_OF_AETHER;
        case GlyphProfiles::BehaviorRivals2: return MODE_RIVALS2;
        case GlyphProfiles::BehaviorSmash64: return MODE_64;
        default:
            break;
    }
    if (layout == GlyphProfiles::Layout::Fgc || layout == GlyphProfiles::Layout::SplitFgc) {
        return MODE_FGC;
    }
    switch (modProfileId) {
        case 2: return MODE_PROJECT_M;
        case 3: return MODE_ULTIMATE;
        case 1:
        case 4:
        case 5:
        default:
            return MODE_MELEE;
    }
}

uint32_t layoutToHaybox(GlyphProfiles::Layout layout)
{
    switch (layout) {
        case GlyphProfiles::Layout::Fgc: return LAYOUT_PLATE_FGC;
        case GlyphProfiles::Layout::SplitFgc: return LAYOUT_PLATE_SPLIT_FGC;
        case GlyphProfiles::Layout::Platform:
        default:
            return LAYOUT_PLATE_PLATFORM_FIGHTER;
    }
}

uint32_t socdToHaybox(SOCDMode mode)
{
    switch (mode) {
        case SOCD_MODE_SECOND_INPUT_PRIORITY: return SOCD_2IP;
        case SOCD_MODE_FIRST_INPUT_PRIORITY: return SOCD_DIR1_PRIORITY;
        case SOCD_MODE_NEUTRAL:
        default:
            return SOCD_NEUTRAL;
    }
}

SOCDMode socdFromHaybox(uint32_t mode)
{
    switch (mode) {
        case SOCD_2IP:
            return SOCD_MODE_SECOND_INPUT_PRIORITY;
        case SOCD_DIR1_PRIORITY:
            return SOCD_MODE_FIRST_INPUT_PRIORITY;
        case SOCD_NEUTRAL:
        default:
            return SOCD_MODE_NEUTRAL;
    }
}

bool decodeString(const uint8_t* data, size_t length, char* destination, size_t destinationLength)
{
    if (destinationLength == 0) {
        return false;
    }
    const size_t copyLength = length < destinationLength - 1 ? length : destinationLength - 1;
    memcpy(destination, data, copyLength);
    destination[copyLength] = '\0';
    return true;
}

bool parsePackedBackends(const uint8_t* data, size_t length, uint16_t& backends)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    while (cursor < end) {
        uint32_t backend = 0;
        if (!readVarint(cursor, end, backend)) {
            return false;
        }
        backends |= backendMaskFromHaybox(backend);
    }
    return true;
}

bool parseSocdPair(const uint8_t* data, size_t length, GlyphProfiles::SocdPair& pair, SOCDMode& socdMode)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    uint32_t buttonDir1 = 0;
    uint32_t buttonDir2 = 0;
    uint32_t socdType = 0;
    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;
        if (field == 1 && wireType == 0) {
            if (!readVarint(cursor, end, buttonDir1)) return false;
        } else if (field == 2 && wireType == 0) {
            if (!readVarint(cursor, end, buttonDir2)) return false;
        } else if (field == 3 && wireType == 0) {
            if (!readVarint(cursor, end, socdType)) return false;
            if (socdType != 0) {
                socdMode = socdFromHaybox(socdType);
            }
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }
    pair = {
        static_cast<uint8_t>(buttonDir1),
        static_cast<uint8_t>(buttonDir2),
        static_cast<uint8_t>(socdType),
    };
    return pair.buttonDir1 != 0 && pair.buttonDir2 != 0;
}

bool parseButtonRemap(const uint8_t* data, size_t length, GlyphProfiles::ButtonRemap& remap)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    uint32_t physicalButton = 0;
    uint32_t targetButton = 0;
    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;
        if (field == 1 && wireType == 0) {
            if (!readVarint(cursor, end, physicalButton)) return false;
        } else if (field == 2 && wireType == 0) {
            if (!readVarint(cursor, end, targetButton)) return false;
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }
    remap = {
        static_cast<uint8_t>(physicalButton),
        static_cast<uint8_t>(targetButton),
    };
    return remap.physicalButton != 0;
}

bool parseButtonColor(const uint8_t* data, size_t length, GlyphProfiles::RgbColor& color)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    uint32_t button = 0;
    uint32_t rgb = 0;
    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;
        if (field == 1 && wireType == 0) {
            if (!readVarint(cursor, end, button)) return false;
        } else if (field == 2 && wireType == 0) {
            if (!readVarint(cursor, end, rgb)) return false;
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }
    color = {
        static_cast<uint8_t>(button),
        rgb & 0x00ffffff,
    };
    return color.button != 0;
}

bool parseRgbConfig(const uint8_t* data, size_t length, WebRgbConfig& config)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    config.colorCount = 0;
    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;
        if (field == 1 && wireType == 2) {
            uint32_t messageLength = 0;
            if (!readVarint(cursor, end, messageLength) || static_cast<size_t>(end - cursor) < messageLength) return false;
            GlyphProfiles::RgbColor color = {};
            if (parseButtonColor(cursor, messageLength, color) && config.colorCount < GlyphProfiles::MaxRgbColors) {
                config.colors[config.colorCount++] = color;
            }
            cursor += messageLength;
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }
    return true;
}

bool parseGameMode(const uint8_t* data, size_t length, uint8_t profileNumber)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    char name[GlyphProfiles::ProfileNameLength + 1] = {};
    uint32_t mode = 0;
    uint32_t layout = 0;
    uint8_t rgbConfig = 0;
    uint16_t backends = 0;
    SOCDMode socdMode = GlyphProfiles::socdMode(profileNumber);
    GlyphProfiles::SocdPair socdPairs[GlyphProfiles::MaxSocdPairs] = {};
    GlyphProfiles::ButtonRemap buttonRemaps[GlyphProfiles::MaxButtonRemaps] = {};
    uint8_t socdPairCount = 0;
    uint8_t buttonRemapCount = 0;
    bool hasMode = false;
    bool hasLayout = false;
    bool hasName = false;
    bool hasRgbConfig = false;
    bool hasBackends = false;

    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;

        if (field == 1 && wireType == 0) {
            if (!readVarint(cursor, end, mode)) return false;
            hasMode = true;
        } else if (field == 2 && wireType == 2) {
            uint32_t stringLength = 0;
            if (!readVarint(cursor, end, stringLength) || static_cast<size_t>(end - cursor) < stringLength) return false;
            decodeString(cursor, stringLength, name, sizeof(name));
            cursor += stringLength;
            hasName = true;
        } else if (field == 3 && wireType == 2) {
            uint32_t messageLength = 0;
            if (!readVarint(cursor, end, messageLength) || static_cast<size_t>(end - cursor) < messageLength) return false;
            GlyphProfiles::SocdPair pair = {};
            if (parseSocdPair(cursor, messageLength, pair, socdMode) && socdPairCount < GlyphProfiles::MaxSocdPairs) {
                socdPairs[socdPairCount++] = pair;
            }
            cursor += messageLength;
        } else if (field == 4 && wireType == 2) {
            uint32_t messageLength = 0;
            if (!readVarint(cursor, end, messageLength) || static_cast<size_t>(end - cursor) < messageLength) return false;
            GlyphProfiles::ButtonRemap remap = {};
            if (parseButtonRemap(cursor, messageLength, remap) && buttonRemapCount < GlyphProfiles::MaxButtonRemaps) {
                buttonRemaps[buttonRemapCount++] = remap;
            }
            cursor += messageLength;
        } else if (field == 8 && wireType == 0) {
            uint32_t value = 0;
            if (!readVarint(cursor, end, value)) return false;
            rgbConfig = static_cast<uint8_t>(value);
            hasRgbConfig = true;
        } else if (field == 200 && wireType == 0) {
            if (!readVarint(cursor, end, layout)) return false;
            hasLayout = true;
        } else if (field == 201 && wireType == 0) {
            uint32_t backend = 0;
            if (!readVarint(cursor, end, backend)) return false;
            backends |= backendMaskFromHaybox(backend);
            hasBackends = true;
        } else if (field == 201 && wireType == 2) {
            uint32_t packedLength = 0;
            if (!readVarint(cursor, end, packedLength) || static_cast<size_t>(end - cursor) < packedLength) return false;
            if (!parsePackedBackends(cursor, packedLength, backends)) return false;
            hasBackends = true;
            cursor += packedLength;
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }

    if (socdPairCount > 0) {
        bool allOff = true;
        bool sawMode = false;
        bool allSameMode = true;
        SOCDMode firstMode = SOCD_MODE_BYPASS;

        for (uint8_t index = 0; index < socdPairCount; index++) {
            if (socdPairs[index].socdType == 0) {
                continue;
            }

            const SOCDMode pairMode = socdFromHaybox(socdPairs[index].socdType);
            allOff = false;
            if (!sawMode) {
                firstMode = pairMode;
                sawMode = true;
            } else if (firstMode != pairMode) {
                allSameMode = false;
            }
        }

        if (allOff) {
            socdMode = SOCD_MODE_BYPASS;
        } else if (sawMode && allSameMode) {
            socdMode = firstMode;
        }
    }

    normalizeImportedRivals2Defaults(mode, buttonRemaps, buttonRemapCount);

    const GlyphProfiles::Layout importedLayout = (hasMode || hasLayout) ?
        layoutFromHaybox(mode, layout) : GlyphProfiles::layout(profileNumber);
    const uint32_t currentBehavior = GlyphProfiles::behaviorMode(profileNumber);
    const uint32_t importedBehavior = hasMode ?
        behaviorFromHaybox(mode, importedLayout, currentBehavior) : currentBehavior;

    if (hasName) {
        GlyphProfiles::setName(profileNumber, name);
    }
    if (hasMode || hasLayout) {
        GlyphProfiles::setLayout(profileNumber, importedLayout);
    }
    if (hasMode) {
        GlyphProfiles::setBehaviorMode(profileNumber, importedBehavior);
    }
    GlyphProfiles::setSOCDMode(profileNumber, socdMode);
    if (hasRgbConfig) {
        GlyphProfiles::setRgbConfig(profileNumber, rgbConfig);
    }
    if (hasBackends) {
        GlyphProfiles::setBackends(profileNumber, backends);
    }
    GlyphProfiles::clearSocdPairs(profileNumber);
    for (uint8_t index = 0; index < socdPairCount; index++) {
        GlyphProfiles::addSocdPair(profileNumber,
                                   socdPairs[index].buttonDir1,
                                   socdPairs[index].buttonDir2,
                                   socdPairs[index].socdType);
    }
    GlyphProfiles::clearButtonRemaps(profileNumber);
    for (uint8_t index = 0; index < buttonRemapCount; index++) {
        const uint8_t targetButton =
            translateImportedRemapTarget(mode, layout, profileNumber, buttonRemaps[index].targetButton);
        GlyphProfiles::addButtonRemap(profileNumber,
                                      buttonRemaps[index].physicalButton,
                                      targetButton);
    }
    return true;
}

bool parseConfig(const uint8_t* data, size_t length)
{
    const uint8_t* cursor = data;
    const uint8_t* end = data + length;
    uint8_t profileNumber = 1;
    uint8_t rgbConfigCount = 0;
    memset(parsedRgbConfigs, 0, sizeof(parsedRgbConfigs));

    while (cursor < end) {
        uint32_t tag = 0;
        if (!readVarint(cursor, end, tag)) {
            return false;
        }
        const uint32_t field = tag >> 3;
        const uint8_t wireType = tag & 0x07;

        if (field == 1 && wireType == 2) {
            uint32_t messageLength = 0;
            if (!readVarint(cursor, end, messageLength) || static_cast<size_t>(end - cursor) < messageLength) return false;
            if (profileNumber <= GlyphProfiles::count()) {
                if (!parseGameMode(cursor, messageLength, profileNumber)) return false;
                profileNumber++;
            }
            cursor += messageLength;
        } else if (field == 5 && wireType == 2) {
            uint32_t messageLength = 0;
            if (!readVarint(cursor, end, messageLength) || static_cast<size_t>(end - cursor) < messageLength) return false;
            if (rgbConfigCount < kMaxWebRgbConfigs) {
                if (!parseRgbConfig(cursor, messageLength, parsedRgbConfigs[rgbConfigCount])) return false;
                rgbConfigCount++;
            }
            cursor += messageLength;
        } else if (!skipField(cursor, end, wireType)) {
            return false;
        }
    }

    const uint8_t importedProfileCount = profileNumber > 0 ? static_cast<uint8_t>(profileNumber - 1) : 0;
    if (rgbConfigCount > 0) {
        for (uint8_t profile = 1; profile <= importedProfileCount; profile++) {
            const uint8_t rgbConfig = GlyphProfiles::rgbConfig(profile);
            if (rgbConfig == 0 || rgbConfig > rgbConfigCount) {
                continue;
            }
            const WebRgbConfig& source = parsedRgbConfigs[rgbConfig - 1];
            GlyphProfiles::clearRgbColors(profile);
            for (uint8_t color = 0; color < source.colorCount; color++) {
                GlyphProfiles::addRgbColor(profile, source.colors[color].button, source.colors[color].color);
            }
        }
    }

    for (uint8_t profile = importedProfileCount + 1; profile <= GlyphProfiles::count(); profile++) {
        GlyphProfiles::restoreProfileDefaults(profile);
    }
    return true;
}

std::vector<uint8_t> encodeDeviceInfo()
{
    std::vector<uint8_t> message;
    writeString(message, 1, "FW-Glyph");
    writeString(message, 2, "gp2040-port");
    writeString(message, 3, "GlyphMK6");
    return message;
}

std::vector<uint8_t> encodeSocdPair(const GlyphProfiles::SocdPair& source)
{
    std::vector<uint8_t> pair;
    writeUInt(pair, 1, source.buttonDir1);
    writeUInt(pair, 2, source.buttonDir2);
    writeUIntAllowZero(pair, 3, source.socdType);
    return pair;
}

std::vector<uint8_t> encodeButtonRemap(uint8_t physicalButton, uint8_t targetButton)
{
    std::vector<uint8_t> remap;
    writeUInt(remap, 1, physicalButton);
    writeUInt(remap, 2, targetButton);
    return remap;
}

std::vector<uint8_t> encodeButtonColor(const GlyphProfiles::RgbColor& source)
{
    std::vector<uint8_t> color;
    writeUInt(color, 1, source.button);
    writeUInt(color, 2, source.color);
    return color;
}

std::vector<uint8_t> encodeRgbConfig(uint8_t profileNumber)
{
    std::vector<uint8_t> config;
    const GlyphProfiles::ProfileState& profile = GlyphProfiles::state(profileNumber);
    for (uint8_t colorIndex = 0; colorIndex < profile.rgbColorCount; colorIndex++) {
        writeMessage(config, 1, encodeButtonColor(profile.rgbColors[colorIndex]));
    }
    writeUInt(config, 3, 1);
    return config;
}

std::vector<uint8_t> encodeGameMode(uint8_t profileNumber)
{
    const GlyphProfiles::ProfileState& profile = GlyphProfiles::state(profileNumber);
    std::vector<uint8_t> mode;
    const uint32_t outgoingMode = modeFromBehavior(profile.behaviorMode, profile.layout, profile.modProfileId);
    writeUInt(mode, 1, outgoingMode);
    writeString(mode, 2, profile.name);
    for (uint8_t pairIndex = 0; pairIndex < profile.socdPairCount; pairIndex++) {
        writeMessage(mode, 3, encodeSocdPair(profile.socdPairs[pairIndex]));
    }
    for (uint8_t remapIndex = 0; remapIndex < profile.buttonRemapCount; remapIndex++) {
        writeMessage(
            mode,
            4,
            encodeButtonRemap(
                profile.buttonRemaps[remapIndex].physicalButton,
                translateExportedRemapTarget(outgoingMode, profile.buttonRemaps[remapIndex].targetButton)
            )
        );
    }
    writeUInt(mode, 8, profile.rgbConfig);
    writeUInt(mode, 200, layoutToHaybox(profile.layout));

    const uint16_t backendMasks[] = {
        GlyphProfiles::BackendDInput,
        GlyphProfiles::BackendXInput,
        GlyphProfiles::BackendGameCube,
        GlyphProfiles::BackendN64,
        GlyphProfiles::BackendSwitch,
        GlyphProfiles::BackendPS4,
        GlyphProfiles::BackendPS5,
    };
    for (const uint16_t backendMask : backendMasks) {
        if ((profile.backends & backendMask) != 0) {
            writeUInt(mode, 201, hayboxBackendFromMask(backendMask));
        }
    }
    return mode;
}

std::vector<uint8_t> encodeConfig()
{
    std::vector<uint8_t> config;
    for (uint8_t profileNumber = 1; profileNumber <= GlyphProfiles::count(); profileNumber++) {
        writeMessage(config, 1, encodeGameMode(profileNumber));
    }
    for (uint8_t profileNumber = 1; profileNumber <= GlyphProfiles::count(); profileNumber++) {
        writeMessage(config, 5, encodeRgbConfig(profileNumber));
    }
    writeUInt(config, 6, 1);
    writeUInt(config, 7, 1);
    return config;
}

size_t cobsDecode(const uint8_t* input, size_t length, uint8_t* output, size_t outputCapacity)
{
    size_t readIndex = 0;
    size_t writeIndex = 0;
    while (readIndex < length) {
        const uint8_t code = input[readIndex++];
        if (code == 0 || writeIndex + code - 1 > outputCapacity) {
            return 0;
        }
        for (uint8_t i = 1; i < code; i++) {
            if (readIndex >= length) {
                return 0;
            }
            output[writeIndex++] = input[readIndex++];
        }
        if (code != 0xff && readIndex < length) {
            if (writeIndex >= outputCapacity) {
                return 0;
            }
            output[writeIndex++] = 0;
        }
    }
    return writeIndex;
}

std::vector<uint8_t> cobsEncode(const uint8_t* input, size_t length)
{
    std::vector<uint8_t> output;
    output.reserve(length + 2);

    size_t codeIndex = 0;
    uint8_t code = 1;
    output.push_back(0);

    for (size_t i = 0; i < length; i++) {
        if (input[i] == 0) {
            output[codeIndex] = code;
            codeIndex = output.size();
            output.push_back(0);
            code = 1;
        } else {
            output.push_back(input[i]);
            code++;
            if (code == 0xff) {
                output[codeIndex] = code;
                codeIndex = output.size();
                output.push_back(0);
                code = 1;
            }
        }
    }

    output[codeIndex] = code;
    output.push_back(0);
    return output;
}

void writePacket(Command command, const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> decoded;
    decoded.reserve(payload.size() + 1);
    decoded.push_back(command);
    decoded.insert(decoded.end(), payload.begin(), payload.end());

    const std::vector<uint8_t> encoded = cobsEncode(decoded.data(), decoded.size());
    tud_cdc_write(encoded.data(), encoded.size());
    tud_cdc_write_flush();
}

void writeError(const char* message)
{
    std::vector<uint8_t> payload(message, message + strlen(message));
    writePacket(CMD_ERROR, payload);
}

void handlePacket(const uint8_t* packet, size_t length)
{
    if (length == 0) {
        return;
    }

    const Command command = static_cast<Command>(packet[0]);
    const uint8_t* payload = packet + 1;
    const size_t payloadLength = length - 1;

    switch (command) {
        case CMD_GET_DEVICE_INFO:
            writePacket(CMD_SET_DEVICE_INFO, encodeDeviceInfo());
            break;
        case CMD_GET_CONFIG:
            writePacket(CMD_SET_CONFIG, encodeConfig());
            break;
        case CMD_SET_CONFIG:
            if (!parseConfig(payload, payloadLength)) {
                writeError("Failed to decode config");
                break;
            }
            GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
            writePacket(CMD_SUCCESS, {});
            break;
        case CMD_REBOOT_FIRMWARE:
        case CMD_REBOOT_BOOTLOADER:
            writePacket(CMD_SUCCESS, {});
            break;
        default:
            writeError("Unknown command");
            break;
    }
}
}

namespace GlyphWebSerial
{
void process()
{
    while (tud_cdc_available() > 0) {
        uint8_t byte = 0;
        tud_cdc_read(&byte, 1);

        if (byte == 0) {
            const size_t decodedLength = cobsDecode(encodedPacket, encodedLength, decodedPacket, sizeof(decodedPacket));
            if (decodedLength > 0) {
                handlePacket(decodedPacket, decodedLength);
            }
            encodedLength = 0;
            continue;
        }

        if (encodedLength < sizeof(encodedPacket)) {
            encodedPacket[encodedLength++] = byte;
        } else {
            encodedLength = 0;
        }
    }
}
}
