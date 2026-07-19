#ifndef _GLYPH_PROFILES_H_
#define _GLYPH_PROFILES_H_

#include "enums.pb.h"
#include "config.pb.h"

#include <stdint.h>

namespace GlyphProfiles
{
constexpr uint8_t MatrixRows = 4;
constexpr uint8_t MatrixCols = 11;
constexpr uint8_t MaxProfiles = 20;
constexpr uint8_t MaxModProfiles = 8;
constexpr uint8_t ProfileNameLength = 16;
constexpr uint8_t MaxSocdPairs = 10;
constexpr uint8_t MaxButtonRemaps = 60;
constexpr uint8_t MaxRgbColors = 60;
constexpr uint8_t LogicalSocdSlotCount = 6;

enum class Layout : uint8_t {
    Platform,
    Fgc,
    SplitFgc,
};

enum class Target : uint8_t {
    None,
    Dpad,
    LeftAnalog,
    RightAnalog,
    Button,
    Aux,
};

struct Action {
    Target target;
    uint32_t mask;
};

enum BackendMask : uint16_t {
    BackendXInput   = 1 << 0,
    BackendDInput   = 1 << 1,
    BackendSwitch   = 1 << 2,
    BackendPS4      = 1 << 3,
    BackendPS5      = 1 << 4,
    BackendGameCube = 1 << 5,
    BackendN64      = 1 << 6,
};

enum class OutputIcon : uint8_t {
    None,
    Home,
    Touchpad,
    XboxBack,
    Start,
    Turbo,
};

constexpr uint32_t BehaviorUnknown = 0;
constexpr uint32_t BehaviorMelee = 1;
constexpr uint32_t BehaviorProjectM = 2;
constexpr uint32_t BehaviorUltimate = 3;
constexpr uint32_t BehaviorFgc = 4;
constexpr uint32_t BehaviorRivals = 5;
constexpr uint32_t BehaviorRivals2 = 6;
constexpr uint32_t BehaviorSmash64 = 8;

enum BehaviorMode : uint32_t {
    BehaviorModeUnknown = BehaviorUnknown,
    BehaviorModeMelee = BehaviorMelee,
    BehaviorModeProjectM = BehaviorProjectM,
    BehaviorModeUltimate = BehaviorUltimate,
    BehaviorModeFgc = BehaviorFgc,
    BehaviorModeRivals = BehaviorRivals,
    BehaviorModeRivals2 = BehaviorRivals2,
    BehaviorModeSmash64 = BehaviorSmash64,
};

struct Profile {
    uint8_t number;
    const char* name;
    uint32_t behaviorMode;
    Layout layout;
    SOCDMode socdMode;
    uint8_t rgbConfig;
    uint8_t modProfileId;
    uint16_t backends;
};

struct ModProfile {
    uint8_t number;
    const char* name;
    uint8_t modXHorizontal;
    uint8_t modXVertical;
    uint8_t modXDiagonalX;
    uint8_t modXDiagonalY;
    uint8_t modYHorizontal;
    uint8_t modYVertical;
    uint8_t modYDiagonalX;
    uint8_t modYDiagonalY;
    uint8_t lightShield1;
    uint8_t lightShield2;
    bool analogTriggersEnabled;
    uint8_t modXX[4];
    uint8_t modXY[4];
    uint8_t modYX[4];
    uint8_t modYY[4];
};

struct ModProfileState {
    uint8_t number;
    char name[ProfileNameLength + 1];
    uint8_t modXHorizontal;
    uint8_t modXVertical;
    uint8_t modXDiagonalX;
    uint8_t modXDiagonalY;
    uint8_t modYHorizontal;
    uint8_t modYVertical;
    uint8_t modYDiagonalX;
    uint8_t modYDiagonalY;
    uint8_t lightShield1;
    uint8_t lightShield2;
    bool analogTriggersEnabled;
    uint8_t modXX[4];
    uint8_t modXY[4];
    uint8_t modYX[4];
    uint8_t modYY[4];
    bool enabled;
};

struct SocdPair {
    uint8_t buttonDir1;
    uint8_t buttonDir2;
    uint8_t socdType;
};

struct ButtonRemap {
    uint8_t physicalButton;
    uint8_t targetButton;
};

struct RgbColor {
    uint8_t button;
    uint32_t color;
};

enum SocdLogicalSlot : uint8_t {
    SocdLogicalSlotDpadHorizontal = 0,
    SocdLogicalSlotDpadVertical = 1,
    SocdLogicalSlotLeftAnalogHorizontal = 2,
    SocdLogicalSlotLeftAnalogVertical = 3,
    SocdLogicalSlotRightAnalogHorizontal = 4,
    SocdLogicalSlotRightAnalogVertical = 5,
};

struct ProfileState {
    uint8_t number;
    char name[ProfileNameLength + 1];
    uint32_t behaviorMode;
    Layout layout;
    SOCDMode socdMode;
    uint8_t rgbConfig;
    uint8_t modProfileId;
    uint16_t backends;
    uint8_t socdPairCount;
    SocdPair socdPairs[MaxSocdPairs];
    uint8_t buttonRemapCount;
    ButtonRemap buttonRemaps[MaxButtonRemaps];
    uint8_t rgbColorCount;
    RgbColor rgbColors[MaxRgbColors];
};

uint8_t count();
const Profile& get(uint8_t profileNumber);
const ProfileState& state(uint8_t profileNumber);
const char* name(uint8_t profileNumber);
uint32_t behaviorMode(uint8_t profileNumber);
Layout layout(uint8_t profileNumber);
SOCDMode socdMode(uint8_t profileNumber);
uint8_t rgbConfig(uint8_t profileNumber);
uint8_t modProfile(uint8_t profileNumber);
uint16_t backends(uint8_t profileNumber);
uint8_t socdPairCount(uint8_t profileNumber);
const SocdPair& socdPair(uint8_t profileNumber, uint8_t index);
uint8_t buttonRemapCount(uint8_t profileNumber);
const ButtonRemap& buttonRemap(uint8_t profileNumber, uint8_t index);
uint8_t rgbColorCount(uint8_t profileNumber);
const RgbColor& rgbColor(uint8_t profileNumber, uint8_t index);
bool rgbColorForButton(uint8_t profileNumber, uint8_t button, uint32_t& color);
const char* logicalSocdSlotLabel(uint8_t profileNumber, uint8_t slot);
bool logicalSocdSlotVisible(uint8_t profileNumber, uint8_t slot);
SOCDMode logicalSocdMode(uint8_t profileNumber, uint8_t slot);
void setLogicalSocdMode(uint8_t profileNumber, uint8_t slot, SOCDMode mode);
void resetToDefaults();
void restoreProfileDefaults(uint8_t profileNumber);
void loadFromConfig(const GlyphOptions& options);
void writeToConfig(GlyphOptions& options);
void setName(uint8_t profileNumber, const char* value);
void setBehaviorMode(uint8_t profileNumber, uint32_t value);
void setLayout(uint8_t profileNumber, Layout value);
void setSOCDMode(uint8_t profileNumber, SOCDMode value);
void setRgbConfig(uint8_t profileNumber, uint8_t value);
void setModProfile(uint8_t profileNumber, uint8_t value);
void setBackends(uint8_t profileNumber, uint16_t value);
void setSocdPairType(uint8_t profileNumber, uint8_t index, uint8_t socdType);
void clearSocdPairs(uint8_t profileNumber);
void addSocdPair(uint8_t profileNumber, uint8_t buttonDir1, uint8_t buttonDir2, uint8_t socdType);
void clearButtonRemaps(uint8_t profileNumber);
void addButtonRemap(uint8_t profileNumber, uint8_t physicalButton, uint8_t targetButton);
void clearRgbColors(uint8_t profileNumber);
void addRgbColor(uint8_t profileNumber, uint8_t button, uint32_t color);
bool isVisibleProfile(uint8_t profileNumber);
bool backendEnabled(uint8_t profileNumber, uint16_t backendMask);
void setBackendEnabled(uint8_t profileNumber, uint16_t backendMask, bool enabled);
bool allowsInputMode(uint8_t profileNumber, InputMode mode);
const char* backendSummary(uint8_t profileNumber);
const char* layoutName(Layout layout);
uint8_t modProfileCount();
const ModProfileState& getModProfile(uint8_t modProfileId);
const char* modProfileName(uint8_t modProfileId);
bool modProfileEditable(uint8_t modProfileId);
bool modProfileVisible(uint8_t modProfileId);
bool duplicateModProfile(uint8_t sourceModProfileId, uint8_t& newModProfileId);
void deleteModProfile(uint8_t modProfileId);
void adjustModProfileCoordinate(uint8_t modProfileId, bool modX, uint8_t slot, bool yAxis, int8_t delta);
void adjustModProfileLightShield(uint8_t modProfileId, bool secondary, int8_t delta);
void setModProfileAnalogTriggersEnabled(uint8_t modProfileId, bool value);
bool modProfileAnalogTriggersEnabled(uint8_t modProfileId);
void restoreModProfileDefaults(uint8_t modProfileId);
uint8_t defaultModProfileForLegacyMode(uint32_t mode);
OutputIcon menuIcon(uint8_t profileNumber, uint8_t menuButtonIndex);
Action buttonAction(uint8_t profileNumber, uint8_t buttonId);
bool buttonAvailable(uint8_t profileNumber, uint8_t buttonId);
uint8_t matrixButton(uint8_t row, uint8_t col);
}

#endif
