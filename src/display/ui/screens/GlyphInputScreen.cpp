#include "display/ui/screens/GlyphInputScreen.h"

#include "display/GPGFX_core.h"
#include "addons/glyph_matrix_input.h"
#include "drivermanager.h"
#include "gamepad.h"
#include "glyph/assets/GlyphButtonBitmaps.h"
#include "glyph/assets/GlyphDashboardBitmaps.h"
#include "glyph/assets/GlyphInputBitmaps.h"
#include "glyph/glyph_profiles.h"
#include "storagemanager.h"

#include <cstring>

namespace
{
constexpr GlyphInputScreen::ButtonDot kFullLayoutDots[] = {
    {"LF4", 6,   29, 4, GAMEPAD_MASK_E1, 0, 0},
    {"LF3", 15,  23, 4, 0, GAMEPAD_MASK_LEFT, 0},
    {"LF2", 25,  22, 4, 0, GAMEPAD_MASK_DOWN, 0},
    {"LF1", 35,  27, 4, GAMEPAD_MASK_E8, 0, 0},
    {"LF5", 24,  32, 4, GAMEPAD_MASK_E7, 0, 0},

    {"RF1", 93,  27, 4, GAMEPAD_MASK_B1, 0, 0},
    {"RF2", 102, 23, 4, GAMEPAD_MASK_B2, 0, 0},
    {"RF3", 112, 24, 4, GAMEPAD_MASK_B3, 0, 0},
    {"RF4", 122, 29, 4, GAMEPAD_MASK_B4, 0, 0},

    {"RF5", 93,  17, 4, GAMEPAD_MASK_R2, 0, 0},
    {"RF6", 102, 13, 4, GAMEPAD_MASK_L1, 0, 0},
    {"RF7", 112, 14, 4, GAMEPAD_MASK_R1, 0, 0},
    {"RF8", 122, 19, 4, GAMEPAD_MASK_L2, 0, 0},
    {"RF9", 101, 34, 4, GAMEPAD_MASK_E9, 0, 0},

    {"LT1", 38,  52, 4, GAMEPAD_MASK_A1, 0, 0},
    {"LT2", 46,  58, 4, GAMEPAD_MASK_A2, 0, 0},
    {"LT3", 46,  46, 4, GAMEPAD_MASK_A3, 0, 0},
    {"LT4", 38,  40, 4, GAMEPAD_MASK_A4, 0, 0},
    {"LT5", 30,  46, 4, GAMEPAD_MASK_L3, 0, 0},
    {"LT6", 59,  50, 5, GAMEPAD_MASK_R3, 0, 0},

    {"RT1", 90,  52, 4, GAMEPAD_MASK_R3, 0, 0},
    {"RT2", 82,  58, 4, GAMEPAD_MASK_R3, 0, 0},
    {"RT3", 82,  46, 4, GAMEPAD_MASK_E3, 0, 0},
    {"RT4", 90,  40, 4, GAMEPAD_MASK_E4, 0, 0},
    {"RT5", 98,  46, 4, GAMEPAD_MASK_E2, 0, 0},

    {"LF8", 35,  17, 4, GAMEPAD_MASK_E5, 0, 0},
    {"LF7", 46,  19, 4, GAMEPAD_MASK_E6, 0, 0},
    {"LF6", 55,  25, 4, 0, GAMEPAD_MASK_RIGHT, 0},

    {"RF10", 64, 30, 4, 0, GAMEPAD_MASK_UP, 0},
    {"RF11", 74, 25, 4, GAMEPAD_MASK_E10, 0, 0},
    {"RF12", 84, 25, 4, GAMEPAD_MASK_E11, 0, 0},
    {"RF13", 64, 20, 4, GAMEPAD_MASK_E12, 0, 0},
    {"RF14", 74, 15, 4, GAMEPAD_MASK_S1, 0, 0},
    {"RF15", 84, 15, 4, GAMEPAD_MASK_S2, 0, 0},
    {"RF16", 72, 35, 4, 0, 0, AUX_MASK_FUNCTION},
};

constexpr GlyphInputScreen::ButtonDot kPlatformLayoutDots[] = {
    {"LF4", 6,   29, 4, GAMEPAD_MASK_E1, 0, 0},
    {"LF3", 15,  23, 4, 0, GAMEPAD_MASK_LEFT, 0},
    {"LF2", 25,  22, 4, 0, GAMEPAD_MASK_DOWN, 0},
    {"LF1", 35,  27, 4, GAMEPAD_MASK_E8, 0, 0},
    {"LF5", 24,  32, 4, GAMEPAD_MASK_E7, 0, 0},
    {"RF1", 93,  27, 4, GAMEPAD_MASK_B1, 0, 0},
    {"RF2", 102, 23, 4, GAMEPAD_MASK_B2, 0, 0},
    {"RF3", 112, 24, 4, GAMEPAD_MASK_B3, 0, 0},
    {"RF4", 122, 29, 4, GAMEPAD_MASK_B4, 0, 0},
    {"RF5", 93,  17, 4, GAMEPAD_MASK_R2, 0, 0},
    {"RF6", 102, 13, 4, GAMEPAD_MASK_L1, 0, 0},
    {"RF7", 112, 14, 4, GAMEPAD_MASK_R1, 0, 0},
    {"RF8", 122, 19, 4, GAMEPAD_MASK_L2, 0, 0},
    {"LT1", 38,  52, 4, GAMEPAD_MASK_A1, 0, 0},
    {"LT2", 46,  58, 4, GAMEPAD_MASK_A2, 0, 0},
    {"LT3", 46,  46, 4, GAMEPAD_MASK_A3, 0, 0},
    {"LT4", 38,  40, 4, GAMEPAD_MASK_A4, 0, 0},
    {"LT5", 30,  46, 4, GAMEPAD_MASK_L3, 0, 0},
    {"LT6", 59,  50, 5, GAMEPAD_MASK_R3, 0, 0},
    {"RT1", 90,  52, 4, GAMEPAD_MASK_R3, 0, 0},
    {"RT2", 82,  58, 4, GAMEPAD_MASK_R3, 0, 0},
    {"RT3", 82,  46, 4, GAMEPAD_MASK_E3, 0, 0},
    {"RT4", 90,  40, 4, GAMEPAD_MASK_E4, 0, 0},
    {"RT5", 98,  46, 4, GAMEPAD_MASK_E2, 0, 0},
};

constexpr GlyphInputScreen::ButtonDot kFgcLayoutDots[] = {
    {"LF8", 35, 17, 4, 0, GAMEPAD_MASK_LEFT, 0},
    {"LF7", 46, 19, 4, 0, GAMEPAD_MASK_DOWN, 0},
    {"LF6", 55, 25, 4, 0, GAMEPAD_MASK_RIGHT, 0},
    {"RF10", 64, 30, 4, 0, GAMEPAD_MASK_UP, 0},
    {"RF11", 74, 25, 4, GAMEPAD_MASK_B1, 0, 0},
    {"RF12", 84, 25, 4, GAMEPAD_MASK_B2, 0, 0},
    {"RF1",  93, 27, 4, GAMEPAD_MASK_B3, 0, 0},
    {"RF13", 64, 20, 4, GAMEPAD_MASK_B4, 0, 0},
    {"RF14", 74, 15, 4, GAMEPAD_MASK_L1, 0, 0},
    {"RF15", 84, 15, 4, GAMEPAD_MASK_R1, 0, 0},
    {"RF5",  93, 17, 4, GAMEPAD_MASK_L2, 0, 0},
    {"RF16", 72, 35, 4, GAMEPAD_MASK_R2, 0, 0},
    {"LT6",  59, 50, 5, GAMEPAD_MASK_A1, 0, 0},
};

constexpr GlyphInputScreen::ButtonDot kSplitFgcLayoutDots[] = {
    {"LF3", 15, 23, 4, 0, GAMEPAD_MASK_LEFT, 0},
    {"LF2", 25, 22, 4, 0, GAMEPAD_MASK_DOWN, 0},
    {"LF1", 35, 27, 4, 0, GAMEPAD_MASK_RIGHT, 0},
    {"LF5", 24, 32, 4, GAMEPAD_MASK_L3, 0, 0},
    {"RF1", 93, 27, 4, GAMEPAD_MASK_B3, 0, 0},
    {"RF2", 102, 23, 4, GAMEPAD_MASK_B1, 0, 0},
    {"RF3", 112, 24, 4, GAMEPAD_MASK_B2, 0, 0},
    {"RF4", 122, 29, 4, GAMEPAD_MASK_R1, 0, 0},
    {"RF5", 93, 17, 4, GAMEPAD_MASK_L2, 0, 0},
    {"RF6", 102, 13, 4, GAMEPAD_MASK_B4, 0, 0},
    {"RF7", 112, 14, 4, GAMEPAD_MASK_L1, 0, 0},
    {"RF8", 122, 19, 4, GAMEPAD_MASK_R2, 0, 0},
    {"RF9", 101, 34, 4, GAMEPAD_MASK_R3, 0, 0},
    {"LT1", 38, 52, 5, GAMEPAD_MASK_A1, 0, 0},
    {"RT1", 90, 52, 5, GAMEPAD_MASK_R3, 0, 0},
};

std::string truncateText(const std::string& text, size_t maxLength)
{
    if (text.length() <= maxLength) {
        return text;
    }

    return text.substr(0, maxLength);
}

int8_t axisToSigned8(uint16_t value)
{
    return static_cast<int8_t>((static_cast<int32_t>(value) - GAMEPAD_JOYSTICK_MID) / 256);
}

void drawGlyphBitmap(GPGFX* renderer, const unsigned char* bitmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y)
{
    renderer->drawSprite((uint8_t*)bitmap, width, height, 0, x, y, 1);
}

void drawRightAlignedText(GPGFX* renderer, uint8_t rightColumn, uint8_t row, const std::string& text)
{
    const uint8_t startColumn = text.length() >= rightColumn ? 0 : rightColumn - text.length() + 1;
    renderer->drawText(startColumn, row, text);
}

void drawHintIcon(GPGFX* renderer, const unsigned char* bitmap, uint16_t x, bool pressed)
{
    if (bitmap == nullptr) {
        return;
    }

    if (pressed) {
        renderer->drawRectangle(x - 1, 47, 14, 14, 1, false);
    }

    drawGlyphBitmap(renderer, bitmap, 12, 12, x, 48);
}

const unsigned char* menuHintIcon(GlyphProfiles::OutputIcon icon)
{
    switch (icon) {
        case GlyphProfiles::OutputIcon::Home:
            return Home12;
        case GlyphProfiles::OutputIcon::XboxBack:
            return XboxBack12;
        case GlyphProfiles::OutputIcon::Start:
            return Start12;
        case GlyphProfiles::OutputIcon::None:
        default:
            return nullptr;
    }
}

bool menuHintPressed(uint8_t menuButtonIndex, const GamepadState& state)
{
    switch (menuButtonIndex) {
        case 4: return (state.aux & AUX_MASK_FUNCTION) != 0;
        case 5: return (state.buttons & GAMEPAD_MASK_S1) != 0;
        case 6: return (state.buttons & GAMEPAD_MASK_S2) != 0;
        default: return false;
    }
}

uint8_t glyphButtonIdFromLabel(const char* label)
{
    if (label == nullptr || label[0] == '\0' || label[1] == '\0') {
        return 0;
    }

    const char hand = label[0];
    const char group = label[1];
    uint8_t value = 0;
    for (const char* cursor = label + 2; *cursor >= '0' && *cursor <= '9'; cursor++) {
        value = static_cast<uint8_t>((value * 10) + (*cursor - '0'));
    }

    if (hand == 'L' && group == 'F' && value >= 1 && value <= 16) return value;
    if (hand == 'R' && group == 'F' && value >= 1 && value <= 16) return static_cast<uint8_t>(16 + value);
    if (hand == 'L' && group == 'T' && value >= 1 && value <= 8) return static_cast<uint8_t>(32 + value);
    if (hand == 'R' && group == 'T' && value >= 1 && value <= 8) return static_cast<uint8_t>(40 + value);
    if (hand == 'M' && group == 'B' && value >= 1 && value <= 12) return static_cast<uint8_t>(48 + value);

    return 0;
}

const unsigned char* activeInputIcon(const GamepadState& state, InputMode mode)
{
    if (GlyphMatrixInput::glyphModXPressed()) return Bitmap_ModX_16;
    if (GlyphMatrixInput::glyphModYPressed()) return Bitmap_ModY_16;

    if (state.buttons & GAMEPAD_MASK_B1) return Bitmap_A_16;
    if (state.buttons & GAMEPAD_MASK_B2) return Bitmap_B_16;
    if (state.buttons & GAMEPAD_MASK_B3) return Bitmap_X_16;
    if (state.buttons & GAMEPAD_MASK_B4) return Bitmap_Y_16;
    if (state.buttons & GAMEPAD_MASK_L1) return mode == INPUT_MODE_SWITCH ? Bitmap_L_16 : Bitmap_LB_16;
    if (state.buttons & GAMEPAD_MASK_R1) return mode == INPUT_MODE_SWITCH ? Bitmap_R_16 : Bitmap_RB_16;
    if (state.buttons & GAMEPAD_MASK_L2) return mode == INPUT_MODE_SWITCH ? Bitmap_ZL_16 : Bitmap_LT_16;
    if (state.buttons & GAMEPAD_MASK_R2) return mode == INPUT_MODE_SWITCH ? Bitmap_ZR_16 : Bitmap_RT_16;
    if (state.buttons & GAMEPAD_MASK_S2) return mode == INPUT_MODE_SWITCH ? Bitmap_Plus_16 : Bitmap_XB_Start_16;
    if (state.buttons & GAMEPAD_MASK_S1) return mode == INPUT_MODE_SWITCH ? Bitmap_Minus_16 : Bitmap_XB_Back_16;
    if (state.aux & AUX_MASK_FUNCTION) return Bitmap_Home_16;
    if (state.buttons & GAMEPAD_MASK_L3) return Bitmap_LSClick_16;
    if (state.buttons & GAMEPAD_MASK_R3) return Bitmap_RSClick_16;

    if (state.dpad & GAMEPAD_MASK_DOWN) return Bitmap_DPadDown_16;
    if (state.dpad & GAMEPAD_MASK_UP) return Bitmap_DpadUp_16;
    if (state.dpad & GAMEPAD_MASK_LEFT) return Bitmap_DPadLeft_16;
    if (state.dpad & GAMEPAD_MASK_RIGHT) return Bitmap_DpadRight_16;

    if (state.lx < (GAMEPAD_JOYSTICK_MID - 4096)) return Bitmap_LS_Left_16;
    if (state.lx > (GAMEPAD_JOYSTICK_MID + 4096)) return Bitmap_LS_Right_16;
    if (state.ly < (GAMEPAD_JOYSTICK_MID - 4096)) return Bitmap_LS_Up_16;
    if (state.ly > (GAMEPAD_JOYSTICK_MID + 4096)) return Bitmap_LS_Down_16;
    if (state.rx < (GAMEPAD_JOYSTICK_MID - 4096)) return Bitmap_RS_Left_16;
    if (state.rx > (GAMEPAD_JOYSTICK_MID + 4096)) return Bitmap_RS_Right_16;
    if (state.ry < (GAMEPAD_JOYSTICK_MID - 4096)) return Bitmap_RS_Up_16;
    if (state.ry > (GAMEPAD_JOYSTICK_MID + 4096)) return Bitmap_RS_Down_16;

    return nullptr;
}
}

bool GlyphInputScreen::inputViewerMode = false;

void GlyphInputScreen::setInputViewerMode(bool enabled)
{
    inputViewerMode = enabled;
}

void GlyphInputScreen::init()
{
    gamepad = Storage::getInstance().GetGamepad();
}

void GlyphInputScreen::shutdown()
{
}

int8_t GlyphInputScreen::update()
{
    return -1;
}

void GlyphInputScreen::drawScreen()
{
    if (gamepad == nullptr) {
        gamepad = Storage::getInstance().GetGamepad();
    }

    const GamepadState state = gamepad != nullptr ? gamepad->state : GamepadState{};
    const InputMode mode = DriverManager::getInstance().getInputMode();
    const uint8_t profileNumber = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;
    const std::string profile = truncateText(GlyphProfiles::name(profileNumber), 10);
    const std::string layout = truncateText(GlyphProfiles::layoutName(GlyphProfiles::layout(profileNumber)), 10);
    const std::string backend = truncateText(inputModeName(mode), 10);

    drawGlyphBitmap(getRenderer(), Bitmap_Dashboard_Base_V3, 128, 64, 0, 0);

    getRenderer()->drawText(2, 1, "Glyph");
    getRenderer()->drawText(2, 2, profile);
    getRenderer()->drawText(2, 3, layout);
    getRenderer()->drawText(2, 4, backend);

    drawRightAlignedText(getRenderer(), 15, 1, std::to_string(axisToSigned8(state.lx)));
    drawRightAlignedText(getRenderer(), 15, 2, std::to_string(axisToSigned8(state.ly)));
    drawRightAlignedText(getRenderer(), 15, 3, std::to_string(axisToSigned8(state.rx)));
    drawRightAlignedText(getRenderer(), 15, 4, std::to_string(axisToSigned8(state.ry)));
    drawRightAlignedText(getRenderer(), 21, 3, std::to_string(state.rt));
    drawRightAlignedText(getRenderer(), 21, 4, std::to_string(state.lt));

    const unsigned char* currentInput = activeInputIcon(state, mode);
    if (currentInput != nullptr) {
        drawGlyphBitmap(getRenderer(), currentInput, 16, 16, 109, 3);
    }

    if (inputViewerMode) {
        size_t dotCount = 0;
        const ButtonDot* dots = activeDots(dotCount);
        for (size_t i = 0; i < dotCount; i++) {
            drawDot(dots[i], state);
        }
    }

    constexpr uint16_t kHintX[] = {4, 22, 40, 58, 76, 94, 112};
    for (uint8_t i = 1; i < 7; i++) {
        drawHintIcon(getRenderer(), menuHintIcon(GlyphProfiles::menuIcon(profileNumber, i)), kHintX[i], menuHintPressed(i, state));
    }
}

void GlyphInputScreen::drawDot(const ButtonDot& dot, const GamepadState& state)
{
    const bool pressed = dotPressed(dot, state);
    getRenderer()->drawEllipse(dot.x, dot.y, dot.radius, dot.radius, 1, pressed);
}

bool GlyphInputScreen::dotPressed(const ButtonDot& dot, const GamepadState& state) const
{
    const uint8_t glyphButtonId = glyphButtonIdFromLabel(dot.label);
    if (glyphButtonId != 0) {
        return GlyphMatrixInput::glyphButtonPressed(glyphButtonId);
    }

    return ((dot.buttonMask != 0 && (state.buttons & dot.buttonMask) != 0) ||
            (dot.dpadMask != 0 && (state.dpad & dot.dpadMask) != 0) ||
            (dot.auxMask != 0 && (state.aux & dot.auxMask) != 0));
}

std::string GlyphInputScreen::inputModeName(InputMode mode) const
{
    switch (mode) {
        case INPUT_MODE_XINPUT: return "XInput";
        case INPUT_MODE_SWITCH: return "Switch";
        case INPUT_MODE_SWITCH_PRO: return "SW Pro";
        case INPUT_MODE_PS3: return "PS3";
        case INPUT_MODE_PS4: return "PS4";
        case INPUT_MODE_PS5: return "PS5";
        case INPUT_MODE_XBONE: return "XBOne";
        case INPUT_MODE_KEYBOARD: return "HID-KB";
        case INPUT_MODE_CONFIG: return "Config";
        default: return "USB";
    }
}

std::string GlyphInputScreen::socdName(SOCDMode mode) const
{
    switch (mode) {
        case SOCD_MODE_NEUTRAL: return "N";
        case SOCD_MODE_UP_PRIORITY: return "U";
        case SOCD_MODE_SECOND_INPUT_PRIORITY: return "L";
        case SOCD_MODE_FIRST_INPUT_PRIORITY: return "F";
        case SOCD_MODE_BYPASS: return "X";
        default: return "?";
    }
}

const GlyphInputScreen::ButtonDot* GlyphInputScreen::activeDots(size_t& count) const
{
    const uint8_t profile = gamepad != nullptr ? gamepad->getOptions().profileNumber : 1;

    switch (GlyphProfiles::layout(profile)) {
        case GlyphProfiles::Layout::Fgc:
            count = sizeof(kFgcLayoutDots) / sizeof(kFgcLayoutDots[0]);
            return kFgcLayoutDots;
        case GlyphProfiles::Layout::SplitFgc:
            count = sizeof(kSplitFgcLayoutDots) / sizeof(kSplitFgcLayoutDots[0]);
            return kSplitFgcLayoutDots;
        case GlyphProfiles::Layout::Platform:
        default:
            count = sizeof(kFullLayoutDots) / sizeof(kFullLayoutDots[0]);
            return kFullLayoutDots;
    }
}
