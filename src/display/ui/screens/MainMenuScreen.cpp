#include "MainMenuScreen.h"
#include "glyph/assets/GlyphButtonBitmaps.h"
#include "glyph/assets/GlyphMenuBitmaps.h"
#include "glyph/glyph_profiles.h"
#include "glyph/glyph_usb_host.h"
#include "display/ui/screens/GlyphInputScreen.h"
#include "hardware/watchdog.h"
#include "system.h"

#include <cstring>
#include <utility>

extern uint32_t getMillis();

#ifdef GLYPH_DISPLAY_SCREEN
namespace
{
constexpr int32_t kGlyphModProfileActionAdd = -101;
constexpr int32_t kGlyphModProfileActionDelete = -102;
constexpr int32_t kGlyphModProfileActionEditModXCRight = -103;
constexpr int32_t kGlyphModProfileActionEditModXCUp = -104;
constexpr int32_t kGlyphModProfileActionEditModXCLeft = -105;
constexpr int32_t kGlyphModProfileActionEditModXCDown = -106;
constexpr int32_t kGlyphModProfileActionEditModYCRight = -107;
constexpr int32_t kGlyphModProfileActionEditModYCUp = -108;
constexpr int32_t kGlyphModProfileActionEditModYCLeft = -109;
constexpr int32_t kGlyphModProfileActionEditModYCDown = -110;
constexpr int32_t kGlyphModProfileActionCoordinateX = -111;
constexpr int32_t kGlyphModProfileActionCoordinateY = -112;
constexpr int32_t kGlyphModProfileActionRestore = -113;
constexpr int32_t kGlyphModProfileActionEditModXBase = -114;
constexpr int32_t kGlyphModProfileActionEditModYBase = -115;
constexpr int32_t kGlyphModProfileActionLightShield1 = -116;
constexpr int32_t kGlyphModProfileActionLightShield2 = -117;
constexpr int32_t kGlyphModProfileActionEditModXDiagonal = -118;
constexpr int32_t kGlyphModProfileActionEditModYDiagonal = -119;
constexpr int32_t kGlyphModProfileActionSingleValue = -120;
constexpr int32_t kGlyphModProfileActionEditModXMagnitudes = -121;
constexpr int32_t kGlyphModProfileActionEditModYMagnitudes = -122;

std::pair<std::string, std::string> glyphTopLabelLines(const std::string& label)
{
    if (label == "Input Viewer") return {"Input", "Viewer"};
    if (label == "Input Mode" || label == "USB Mode") return {"USB", "Mode"};
    if (label == "USB Host") return {"USB", "Host"};
    if (label == "USB Support") return {"USB", "Support"};
    if (label == "Profiles") return {"Profile", ""};
    if (label == "Mod Profile") return {"Mod", "Profile"};
    if (label == "Analog Triggers") return {"Analog", "Triggers"};
    if (label == "Light Shields") return {"Light", "Shields"};
    if (label == "Magnitudes") return {"Magnitude", ""};
    if (label == "D-Pad Mode") return {"D-Pad", "Mode"};
    if (label == "SOCD Mode") return {"SOCD", "Mode"};
    if (label == "RGB Brightness") return {"RGB", "Bright"};
    if (label == "Turbo") return {"Turbo", ""};
    if (label == "Configurator") return {"Config", ""};
    if (label == "FW Update") return {"Bootsel", ""};
    if (label == "Exit") return {"Exit", ""};
    return {label, ""};
}

const char* glyphIconText(const std::string& label)
{
    if (label == "Input Viewer") return "IN";
    if (label == "Input Mode" || label == "USB Mode") return "USB";
    if (label == "USB Host") return "USB";
    if (label == "USB Support") return "USB";
    if (label == "Profiles") return "PRO";
    if (label == "Mod Profile") return "MOD";
    if (label == "Analog Triggers") return "AT";
    if (label == "Light Shields") return "LS";
    if (label == "Magnitudes") return "MAG";
    if (label == "D-Pad Mode") return "DP";
    if (label == "SOCD Mode") return "SO";
    if (label == "RGB Brightness") return "RGB";
    if (label == "Turbo") return "TB";
    if (label == "Configurator") return "CFG";
    if (label == "FW Update") return "FW";
    if (label == "Exit") return "EX";
    return "--";
}

const unsigned char* glyphSmallIcon(const std::string& label)
{
    if (label == "Input Viewer") return bitmap_input_small;
    if (label == "Input Mode" || label == "USB Mode") return bitmap_usb_small;
    if (label == "USB Host") return bitmap_usb_small;
    if (label == "USB Support") return bitmap_usb_small;
    if (label == "Profiles") return bitmap_gamemode_small;
    if (label == "Mod Profile") return bitmap_profile_small;
    if (label == "Analog Triggers") return bitmap_profile_small;
    if (label == "Light Shields") return bitmap_profile_small;
    if (label == "Magnitudes") return bitmap_profile_small;
    if (label == "D-Pad Mode") return bitmap_input_small;
    if (label == "SOCD Mode") return bitmap_profile_small;
    if (label == "RGB Brightness") return bitmap_brightness_small;
    if (label == "Turbo") return bitmap_profile_small;
    if (label == "Configurator") return bitmap_config_small;
    if (label == "FW Update") return bitmap_firmware_small;
    if (label == "Exit") return bitmap_save_small;
    return bitmap_about_small;
}

const unsigned char* glyphLargeIcon(const std::string& label)
{
    if (label == "Input Viewer") return bitmap_input_large;
    if (label == "Input Mode" || label == "USB Mode") return bitmap_usb_large;
    if (label == "USB Host") return bitmap_usb_large;
    if (label == "USB Support") return bitmap_usb_large;
    if (label == "Profiles") return bitmap_gamemode_large;
    if (label == "Mod Profile") return bitmap_profile_large;
    if (label == "Analog Triggers") return bitmap_profile_large;
    if (label == "Light Shields") return bitmap_profile_large;
    if (label == "Magnitudes") return bitmap_profile_large;
    if (label == "D-Pad Mode") return bitmap_input_large;
    if (label == "SOCD Mode") return bitmap_profile_large;
    if (label == "RGB Brightness") return bitmap_brightness_large;
    if (label == "Turbo") return bitmap_profile_large;
    if (label == "Configurator") return bitmap_config_large;
    if (label == "FW Update") return bitmap_firmware_large;
    if (label == "Exit") return bitmap_save_large;
    return bitmap_about_large;
}

void drawGlyphBitmap(GPGFX* renderer, const unsigned char* bitmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y)
{
    renderer->drawSprite((uint8_t*)bitmap, width, height, 0, x, y, 1);
}

void drawGlyphRightAlignedText(GPGFX* renderer, uint8_t rightColumn, uint8_t row, const std::string& text)
{
    const uint8_t startColumn = text.length() >= rightColumn ? 0 : rightColumn - text.length() + 1;
    renderer->drawText(startColumn, row, text);
}

void setGlyphButtonsScreenMode(bool inputViewer)
{
    GlyphInputScreen::setInputViewerMode(inputViewer);
}

std::string glyphListTitle(std::vector<MenuEntry>* menu,
                           std::vector<MenuEntry>* profilesMenu,
                           std::vector<MenuEntry>* modProfileMenu,
                           std::vector<MenuEntry>* analogTriggersMenu,
                           std::vector<MenuEntry>* lightShieldMenu,
                           std::vector<MenuEntry>* modXAngleMenu,
                           std::vector<MenuEntry>* modYAngleMenu,
                           std::vector<MenuEntry>* modMagnitudeMenu,
                           std::vector<MenuEntry>* modCoordinateMenu,
                           std::vector<MenuEntry>* modSingleValueMenu,
                           std::vector<MenuEntry>* inputModeMenu,
                           std::vector<MenuEntry>* usbHostMenu,
                           std::vector<MenuEntry>* backendSupportMenu,
                           std::vector<MenuEntry>* dpadModeMenu,
                           std::vector<MenuEntry>* socdModeMenu,
                           std::vector<MenuEntry>* rgbBrightnessMenu,
                           std::vector<MenuEntry>* turboModeMenu,
                           std::vector<MenuEntry>* saveMenu)
{
    if (menu == profilesMenu) return "Set Profile";
    if (menu == modProfileMenu) return "Mod Profile";
    if (menu == analogTriggersMenu) return "Analog Triggers";
    if (menu == lightShieldMenu) return "Light Shields";
    if (menu == modXAngleMenu) return "Edit Mod X";
    if (menu == modYAngleMenu) return "Edit Mod Y";
    if (menu == modMagnitudeMenu) return "Magnitudes";
    if (menu == modCoordinateMenu) return "";
    if (menu == modSingleValueMenu) return "";
    if (menu == inputModeMenu) return "USB Mode";
    if (menu == usbHostMenu) return "USB Host";
    if (menu == backendSupportMenu) return "USB Support";
    if (menu == dpadModeMenu) return "D-Pad Mode";
    if (menu == socdModeMenu) return "SOCD Mode";
    if (menu == rgbBrightnessMenu) return "RGB Bright";
    if (menu == turboModeMenu) return "Turbo";
    if (menu == saveMenu) return "Exit";
    return "Select";
}

std::pair<std::string, std::string> glyphListTitleLines(const std::string& title)
{
    const size_t split = title.find(' ');
    if (split == std::string::npos) {
        return {title, ""};
    }

    return {title.substr(0, split), title.substr(split + 1)};
}

std::string glyphMenuLabelText(const std::string& label, bool selected, size_t maxLength)
{
    if (label.length() <= maxLength) {
        return label;
    }

    if (!selected) {
        return label.substr(0, maxLength);
    }

    const std::string padded = label + "   ";
    const size_t windowCount = padded.length();
    const size_t offset = (getMillis() / 350) % windowCount;

    std::string window;
    window.reserve(maxLength);
    for (size_t i = 0; i < maxLength; i++) {
        window.push_back(padded[(offset + i) % windowCount]);
    }
    return window;
}

}
#endif

void MainMenuScreen::init() {
    getRenderer()->clearScreen();
    currentMenu = &mainMenu;
    previousMenu = nullptr;
    menuHistory.clear();

    exitToScreen = -1;

    gpMenu = new GPMenu();
    gpMenu->setRenderer(getRenderer());
    gpMenu->setPosition(8, 16);
    gpMenu->setStrokeColor(1);
    gpMenu->setFillColor(1);
    gpMenu->setMenuSize(18, menuLineSize);
    gpMenu->setViewport(this->getViewport());
    gpMenu->setShape(GPShape_Type::GP_SHAPE_SQUARE);
    gpMenu->setMenuData(currentMenu);
    gpMenu->setMenuTitle(MAIN_MENU_NAME);
#ifdef GLYPH_DISPLAY_SCREEN
    // Glyph renders its own menu surface; keep GPMenu as state only.
    gpMenu->setVisibility(false);
#endif
    addElement(gpMenu);

    mapMenuUp = new GamepadButtonMapping(GAMEPAD_MASK_UP);
    mapMenuDown = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
    mapMenuLeft = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
    mapMenuRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
    mapMenuSelect = new GamepadButtonMapping(GAMEPAD_MASK_B1);
    mapMenuBack = new GamepadButtonMapping(GAMEPAD_MASK_B2);
    mapMenuToggle = new GamepadButtonMapping(0);

#ifdef GLYPH_DISPLAY_SCREEN
    for (uint8_t profileCtr = 1; profileCtr <= GlyphProfiles::count(); profileCtr++) {
        if (!GlyphProfiles::isVisibleProfile(profileCtr)) {
            continue;
        }
        const char* profileName = GlyphProfiles::name(profileCtr);
        MenuEntry menuEntry = {profileName, NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr};
        profilesMenu.push_back(menuEntry);
    }
#else
    // populate the profiles menu
    uint8_t profileCount = (sizeof(Storage::getInstance().getProfileOptions().gpioMappingsSets)/sizeof(GpioMappings))+1;
    for (uint8_t profileCtr = 0; profileCtr < profileCount; profileCtr++) {
        std::string menuLabel = "";
        if (profileCtr == 0) {
            menuLabel = Storage::getInstance().getGpioMappings().profileLabel;
            // If Profile #1 has no name, set it to Profile #1
            if (menuLabel.empty()) {
                menuLabel = "Profile #" + std::to_string(profileCtr+1);
            }
        } else {
            menuLabel = Storage::getInstance().getProfileOptions().gpioMappingsSets[profileCtr-1].profileLabel;
            // Do not show other profiles if they are empty
            if (menuLabel.empty()) {
                continue;
            }
        }
        
        MenuEntry menuEntry = {menuLabel, NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr+1};
        profilesMenu.push_back(menuEntry);
    }
#endif

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        switch (pinMappings[pin].action) {
            case GpioAction::MENU_NAVIGATION_UP: mapMenuUp->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_DOWN: mapMenuDown->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_LEFT: mapMenuLeft->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_RIGHT: mapMenuRight->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_SELECT: mapMenuSelect->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_BACK: mapMenuBack->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_TOGGLE: mapMenuToggle->pinMask |= 1 << pin; break;
            default:    break;
        }
    }

    changeRequiresReboot = false;
    changeRequiresSave = false;
    prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    updateInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    
    prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    updateDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    
    prevSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
    updateSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
    
    prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    updateProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    
    prevFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
    updateFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
    
    prevTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;
    updateTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;

#ifdef GLYPH_DISPLAY_SCREEN
    populateGlyphBackendMenu();
    populateGlyphBackendSupportMenu();
    populateGlyphModProfileMenu();
#endif

    setMenuHome();
}

void MainMenuScreen::shutdown() {
    clearElements();
    exitToScreen = -1;
}

#ifdef GLYPH_DISPLAY_SCREEN
void MainMenuScreen::populateGlyphBackendMenu()
{
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;

    inputModeMenu.clear();

    const struct {
        const char* label;
        InputMode mode;
    } modes[] = {
        {"XInput",     INPUT_MODE_XINPUT},
        {"Xbox One",   INPUT_MODE_XBONE},
        {"DInput",     INPUT_MODE_GENERIC},
        {"Switch",     INPUT_MODE_SWITCH},
        {"Switch Pro", INPUT_MODE_SWITCH_PRO},
        {"PS4",        INPUT_MODE_PS4},
        {"PS5",        INPUT_MODE_PS5},
        {"P5 General", INPUT_MODE_P5GENERAL},
    };

    for (const auto& mode : modes) {
        if (GlyphProfiles::allowsInputMode(profile, mode.mode)) {
            inputModeMenu.push_back({mode.label, NULL, nullptr, std::bind(&MainMenuScreen::currentInputMode, this), std::bind(&MainMenuScreen::selectInputMode, this), mode.mode});
        }
    }

    if (inputModeMenu.empty()) {
        inputModeMenu.push_back({"No GP USB", NULL, nullptr, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this), -1});
    }
}

void MainMenuScreen::populateGlyphBackendSupportMenu()
{
    backendSupportMenu.clear();

    const struct {
        const char* label;
        uint16_t mask;
    } backends[] = {
        {"XInput/XB1", GlyphProfiles::BackendXInput},
        {"DInput", GlyphProfiles::BackendDInput},
        {"Switch", GlyphProfiles::BackendSwitch},
        {"PS4", GlyphProfiles::BackendPS4},
        {"PS5", GlyphProfiles::BackendPS5},
    };

    for (const auto& backend : backends) {
        backendSupportMenu.push_back({backend.label, NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphBackendSupport, this), std::bind(&MainMenuScreen::toggleGlyphBackendSupport, this), backend.mask});
    }
}

void MainMenuScreen::populateGlyphModProfileMenu()
{
    modProfileMenu.clear();
    analogTriggersMenu.clear();
    lightShieldMenu.clear();

    if (GlyphProfiles::modProfileVisible(6)) {
        modProfileMenu.push_back({
            GlyphProfiles::modProfileName(6),
            NULL,
            nullptr,
            std::bind(&MainMenuScreen::currentGlyphModProfile, this),
            std::bind(&MainMenuScreen::selectGlyphModProfile, this),
            6
        });
    }
    for (uint8_t modProfile = 1; modProfile <= GlyphProfiles::modProfileCount(); modProfile++) {
        if (modProfile == 6) {
            continue;
        }
        if (!GlyphProfiles::modProfileVisible(modProfile)) {
            continue;
        }
        modProfileMenu.push_back({
            GlyphProfiles::modProfileName(modProfile),
            NULL,
            nullptr,
            std::bind(&MainMenuScreen::currentGlyphModProfile, this),
            std::bind(&MainMenuScreen::selectGlyphModProfile, this),
            modProfile
        });
    }

    bool hasFreeCustomSlot = false;
    for (uint8_t modProfile = 1; modProfile <= GlyphProfiles::modProfileCount(); modProfile++) {
        if (GlyphProfiles::modProfileEditable(modProfile) && !GlyphProfiles::modProfileVisible(modProfile)) {
            hasFreeCustomSlot = true;
            break;
        }
    }

    if (hasFreeCustomSlot) {
        modProfileMenu.push_back({"Duplicate Current", NULL, nullptr, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionAdd});
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const uint8_t currentModProfile = GlyphProfiles::modProfile(profile);
    analogTriggersMenu.push_back({"Disabled", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphAnalogTriggers, this), std::bind(&MainMenuScreen::selectGlyphAnalogTriggers, this), 0});
    analogTriggersMenu.push_back({"Enabled", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphAnalogTriggers, this), std::bind(&MainMenuScreen::selectGlyphAnalogTriggers, this), 1});
    populateGlyphLightShieldMenu();
    populateGlyphModAngleMenus();

    modProfileMenu.push_back({"Edit Mod X", NULL, &modXAngleMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)});
    modProfileMenu.push_back({"Edit Mod Y", NULL, &modYAngleMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)});
    modProfileMenu.push_back({"Light Shields", NULL, &lightShieldMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)});
    modProfileMenu.push_back({"Analog Triggers", NULL, &analogTriggersMenu, std::bind(&MainMenuScreen::currentGlyphAnalogTriggers, this), std::bind(&MainMenuScreen::testMenu, this)});

    if (GlyphProfiles::modProfileEditable(currentModProfile)) {
        modProfileMenu.push_back({"Delete Current", NULL, nullptr, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionDelete});
    }
    if (!GlyphProfiles::modProfileEditable(currentModProfile)) {
        modProfileMenu.push_back({"Restore Current", NULL, nullptr, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionRestore});
    }
}

void MainMenuScreen::populateGlyphModAngleMenus()
{
    modXAngleMenu.clear();
    modYAngleMenu.clear();
    modMagnitudeMenu.clear();

    modXAngleMenu.push_back({"Magnitudes", NULL, &modMagnitudeMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModXMagnitudes});
    modXAngleMenu.push_back({"C-Right", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModXCRight});
    modXAngleMenu.push_back({"C-Up", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModXCUp});
    modXAngleMenu.push_back({"C-Left", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModXCLeft});
    modXAngleMenu.push_back({"C-Down", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModXCDown});

    modYAngleMenu.push_back({"Magnitudes", NULL, &modMagnitudeMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModYMagnitudes});
    modYAngleMenu.push_back({"C-Right", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModYCRight});
    modYAngleMenu.push_back({"C-Up", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModYCUp});
    modYAngleMenu.push_back({"C-Left", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModYCLeft});
    modYAngleMenu.push_back({"C-Down", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionEditModYCDown});

    modMagnitudeMenu.push_back({"Base", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), modMagnitudeMenuIsModX ? kGlyphModProfileActionEditModXBase : kGlyphModProfileActionEditModYBase});
    modMagnitudeMenu.push_back({"Diagonal", NULL, &modCoordinateMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), modMagnitudeMenuIsModX ? kGlyphModProfileActionEditModXDiagonal : kGlyphModProfileActionEditModYDiagonal});
}

void MainMenuScreen::populateGlyphModCoordinateMenu()
{
    modCoordinateMenu.clear();
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const GlyphProfiles::ModProfileState& modState = GlyphProfiles::getModProfile(GlyphProfiles::modProfile(profile));
    const bool isBase = modCoordinateMenuSlot == 4;
    const bool isDiagonal = modCoordinateMenuSlot == 5;
    const uint8_t x = modCoordinateMenuIsModX ?
        (isBase ? modState.modXHorizontal : (isDiagonal ? modState.modXDiagonalX : modState.modXX[modCoordinateMenuSlot])) :
        (isBase ? modState.modYHorizontal : (isDiagonal ? modState.modYDiagonalX : modState.modYX[modCoordinateMenuSlot]));
    const uint8_t y = modCoordinateMenuIsModX ?
        (isBase ? modState.modXVertical : (isDiagonal ? modState.modXDiagonalY : modState.modXY[modCoordinateMenuSlot])) :
        (isBase ? modState.modYVertical : (isDiagonal ? modState.modYDiagonalY : modState.modYY[modCoordinateMenuSlot]));
    modCoordinateMenu.push_back({"X " + std::to_string(x), NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphModCoordinateValue, this), std::bind(&MainMenuScreen::testMenu, this), kGlyphModProfileActionCoordinateX});
    modCoordinateMenu.push_back({"Y " + std::to_string(y), NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphModCoordinateValue, this), std::bind(&MainMenuScreen::testMenu, this), kGlyphModProfileActionCoordinateY});
}

void MainMenuScreen::populateGlyphLightShieldMenu()
{
    lightShieldMenu.clear();
    lightShieldMenu.push_back({"Shield 1", NULL, &modSingleValueMenu, std::bind(&MainMenuScreen::currentGlyphModSingleValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionLightShield1});
    lightShieldMenu.push_back({"Shield 2", NULL, &modSingleValueMenu, std::bind(&MainMenuScreen::currentGlyphModSingleValue, this), std::bind(&MainMenuScreen::selectGlyphModProfile, this), kGlyphModProfileActionLightShield2});
}

void MainMenuScreen::populateGlyphModSingleValueMenu()
{
    modSingleValueMenu.clear();
    modSingleValueMenu.push_back({"Value " + std::to_string(currentGlyphModSingleValue()), NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphModSingleValue, this), std::bind(&MainMenuScreen::testMenu, this), kGlyphModProfileActionSingleValue});
}

void MainMenuScreen::toggleGlyphBackendSupport()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return;
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const uint16_t backendMask = static_cast<uint16_t>(currentMenu->at(gpMenu->getIndex()).optionValue);
    const bool enabled = GlyphProfiles::backendEnabled(profile, backendMask);

    GlyphProfiles::setBackendEnabled(profile, backendMask, !enabled);
    GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
    populateGlyphBackendMenu();
    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
}

int32_t MainMenuScreen::currentGlyphBackendSupport()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return 0;
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const uint16_t backendMask = static_cast<uint16_t>(currentMenu->at(gpMenu->getIndex()).optionValue);
    return GlyphProfiles::backendEnabled(profile, backendMask) ? backendMask : 0;
}

void MainMenuScreen::selectGlyphModProfile()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return;
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const int32_t selectedValue = currentMenu->at(gpMenu->getIndex()).optionValue;
    uint8_t modProfile = GlyphProfiles::modProfile(profile);

    if (selectedValue == kGlyphModProfileActionAdd) {
        uint8_t newModProfile = modProfile;
        if (GlyphProfiles::duplicateModProfile(modProfile, newModProfile)) {
            GlyphProfiles::setModProfile(profile, newModProfile);
            GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
            populateGlyphModProfileMenu();
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        return;
    }

    if (selectedValue == kGlyphModProfileActionDelete) {
        if (GlyphProfiles::modProfileEditable(modProfile)) {
            GlyphProfiles::deleteModProfile(modProfile);
            GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
            populateGlyphModProfileMenu();
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        return;
    }

    if (selectedValue == kGlyphModProfileActionRestore) {
        if (!GlyphProfiles::modProfileEditable(modProfile)) {
            GlyphProfiles::restoreModProfileDefaults(modProfile);
            GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
            populateGlyphModProfileMenu();
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        return;
    }

    switch (selectedValue) {
        case kGlyphModProfileActionEditModXMagnitudes:
            modMagnitudeMenuIsModX = true;
            populateGlyphModAngleMenus();
            return;
        case kGlyphModProfileActionEditModYMagnitudes:
            modMagnitudeMenuIsModX = false;
            populateGlyphModAngleMenus();
            return;
        case kGlyphModProfileActionEditModXCRight:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 0;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModXBase:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 4;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModXDiagonal:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 5;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModXCUp:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 1;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModXCLeft:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 2;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModXCDown:
            modCoordinateMenuIsModX = true;
            modCoordinateMenuSlot = 3;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYCRight:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 0;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYBase:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 4;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYDiagonal:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 5;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYCUp:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 1;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYCLeft:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 2;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionEditModYCDown:
            modCoordinateMenuIsModX = false;
            modCoordinateMenuSlot = 3;
            modCoordinateEditing = false;
            populateGlyphModCoordinateMenu();
            return;
        case kGlyphModProfileActionLightShield1:
            modSingleValueSource = kGlyphModProfileActionLightShield1;
            modSingleValueTitle = "Shield 1";
            modSingleValueEditing = false;
            populateGlyphModSingleValueMenu();
            return;
        case kGlyphModProfileActionLightShield2:
            modSingleValueSource = kGlyphModProfileActionLightShield2;
            modSingleValueTitle = "Shield 2";
            modSingleValueEditing = false;
            populateGlyphModSingleValueMenu();
            return;
        default:
            break;
    }

    if (selectedValue <= kGlyphModProfileActionCoordinateY) {
        return;
    }

    modProfile = static_cast<uint8_t>(selectedValue);
    if (GlyphProfiles::modProfile(profile) == modProfile) {
        return;
    }

    GlyphProfiles::setModProfile(profile, modProfile);
    GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
}

int32_t MainMenuScreen::currentGlyphModProfile()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue <= 0) {
        return 0;
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    return GlyphProfiles::modProfile(profile);
}

int32_t MainMenuScreen::currentGlyphModCoordinateValue()
{
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const GlyphProfiles::ModProfileState& modState = GlyphProfiles::getModProfile(GlyphProfiles::modProfile(profile));
    const bool useY = currentMenu->at(gpMenu->getIndex()).optionValue == kGlyphModProfileActionCoordinateY;

    if (modCoordinateMenuIsModX) {
        if (modCoordinateMenuSlot == 4) {
            return useY ? modState.modXVertical : modState.modXHorizontal;
        }
        if (modCoordinateMenuSlot == 5) {
            return useY ? modState.modXDiagonalY : modState.modXDiagonalX;
        }
        return useY ? modState.modXY[modCoordinateMenuSlot] : modState.modXX[modCoordinateMenuSlot];
    }

    if (modCoordinateMenuSlot == 4) {
        return useY ? modState.modYVertical : modState.modYHorizontal;
    }
    if (modCoordinateMenuSlot == 5) {
        return useY ? modState.modYDiagonalY : modState.modYDiagonalX;
    }

    return useY ? modState.modYY[modCoordinateMenuSlot] : modState.modYX[modCoordinateMenuSlot];
}

int32_t MainMenuScreen::currentGlyphModSingleValue()
{
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const GlyphProfiles::ModProfileState& modState = GlyphProfiles::getModProfile(GlyphProfiles::modProfile(profile));
    int32_t source = modSingleValueSource;
    if (currentMenu == &lightShieldMenu && !currentMenu->empty()) {
        source = currentMenu->at(gpMenu->getIndex()).optionValue;
    }

    if (source == kGlyphModProfileActionLightShield1) {
        return modState.lightShield1;
    }
    if (source == kGlyphModProfileActionLightShield2) {
        return modState.lightShield2;
    }
    return 0;
}

void MainMenuScreen::selectGlyphAnalogTriggers()
{
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    const bool enabled = currentMenu->at(gpMenu->getIndex()).optionValue != 0;
    GlyphProfiles::setModProfileAnalogTriggersEnabled(GlyphProfiles::modProfile(profile), enabled);
    GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
    populateGlyphModProfileMenu();
    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
}

int32_t MainMenuScreen::currentGlyphAnalogTriggers()
{
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    return GlyphProfiles::modProfileAnalogTriggersEnabled(GlyphProfiles::modProfile(profile)) ? 1 : 0;
}

void MainMenuScreen::refreshGlyphUsbHostMenuLabels()
{
    usbHostMenu.clear();
    if (Storage::getInstance().getPeripheralOptions().blockUSB0.enabled) {
        usbHostMenu.push_back({"Port Disable", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), GlyphUsbHost::OptionDisable});
    }
    usbHostMenu.push_back({"XInput Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), GlyphUsbHost::OptionXInputAuth});
    usbHostMenu.push_back({"Xbox One Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), GlyphUsbHost::OptionXBOneAuth});
    usbHostMenu.push_back({"PS4 Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), GlyphUsbHost::OptionPS4Auth});
    usbHostMenu.push_back({"PS5 Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), GlyphUsbHost::OptionPS5Auth});

    if (gpMenu != nullptr && currentMenu == &usbHostMenu && gpMenu->getIndex() >= usbHostMenu.size()) {
        gpMenu->setIndex(0);
    }
}

void MainMenuScreen::toggleGlyphUsbHostOption()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return;
    }

    const int32_t option = currentMenu->at(gpMenu->getIndex()).optionValue;
    AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();
    GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
    PeripheralOptions& peripheralOptions = Storage::getInstance().getPeripheralOptions();
    const bool changed = GlyphUsbHost::toggleOption(addonOptions, gamepadOptions, peripheralOptions, option);

    if (changed) {
        EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, true));
        exitToScreen = DisplayMode::RESTART;
        exitToScreenBeforePrompt = DisplayMode::RESTART;
    }
}

int32_t MainMenuScreen::currentGlyphUsbHostOption()
{
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return 0;
    }

    const int32_t option = currentMenu->at(gpMenu->getIndex()).optionValue;
    const GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
    const PeripheralOptions& peripheralOptions = Storage::getInstance().getPeripheralOptions();
    return GlyphUsbHost::currentOption(gamepadOptions, peripheralOptions, option);
}
#endif

void MainMenuScreen::drawScreen() {
    if (gpMenu == nullptr) return;
#ifdef GLYPH_DISPLAY_SCREEN
    gpMenu->setVisibility(false);
    drawGlyphMenu();
    return;
#endif
    gpMenu->setVisibility(!screenIsPrompting);

    if (!screenIsPrompting) {

    } else {
        getRenderer()->drawText(1, 1, "Config has changed.");
        if (changeRequiresSave && !changeRequiresReboot) {
            getRenderer()->drawText(3, 3, "Would you like");
            getRenderer()->drawText(6, 4, "to save?");
        } else if (changeRequiresSave && changeRequiresReboot) {
            getRenderer()->drawText(3, 3, "Would you like");
            getRenderer()->drawText(1, 4, "to save & restart?");
        } else {

        }
        
        if (promptChoice) getRenderer()->drawText(5, 6, CHAR_RIGHT);
        getRenderer()->drawText(6, 6, "Yes");
        if (!promptChoice) getRenderer()->drawText(11, 6, CHAR_RIGHT);
        getRenderer()->drawText(12, 6, "No");
    }
}

#ifdef GLYPH_DISPLAY_SCREEN
void MainMenuScreen::drawGlyphMenu()
{
    if (screenIsPrompting) {
        getRenderer()->drawText(1, 1, "Config changed");
        getRenderer()->drawText(2, 3, changeRequiresReboot ? "Save & restart?" : "Save changes?");
        getRenderer()->drawText(6, 6, promptChoice ? "> Yes" : "  Yes");
        getRenderer()->drawText(12, 6, promptChoice ? "  No" : "> No");
        return;
    }

    if (currentMenu == &mainMenu) {
        drawGlyphTopMenu();
    } else {
        drawGlyphListMenu();
    }

    drawGlyphControls();
}

void MainMenuScreen::drawGlyphTopMenu()
{
    const uint16_t menuSize = currentMenu->size();
    const uint16_t index = gpMenu->getIndex();

    drawGlyphBitmap(getRenderer(), bitmap_glyph_menu_base, 128, 64, 0, 0);

    const struct {
        int8_t relativeIndex;
        uint16_t x;
        uint16_t y;
        bool selected;
    } slots[] = {
        {-2,  8,  5, false},
        {-1, 24,  5, false},
        { 0, 48,  1, true},
        { 1, 82,  5, false},
        { 2, 98,  5, false},
    };

    auto wrapIndex = [menuSize](int16_t value) -> uint16_t {
        if (menuSize == 0) {
            return 0;
        }

        while (value < 0) {
            value += menuSize;
        }
        return static_cast<uint16_t>(value % menuSize);
    };

    for (const auto& slot : slots) {
        if (!slot.selected && menuSize <= static_cast<uint16_t>(slot.relativeIndex < 0 ? -slot.relativeIndex : slot.relativeIndex)) {
            continue;
        }

        const uint16_t itemIndex = wrapIndex(static_cast<int16_t>(index) + slot.relativeIndex);
        if (slot.selected) {
            drawGlyphBitmap(getRenderer(), glyphLargeIcon(currentMenu->at(itemIndex).label), 28, 28, slot.x, slot.y);
        } else {
            drawGlyphBitmap(getRenderer(), glyphSmallIcon(currentMenu->at(itemIndex).label), 10, 10, slot.x, slot.y);
        }
    }

    auto labelLines = glyphTopLabelLines(currentMenu->at(index).label);
    std::string firstLine = labelLines.first;
    std::string secondLine = labelLines.second;
    if (firstLine.length() > 9) firstLine.resize(9);
    if (secondLine.length() > 9) secondLine.resize(9);

    getRenderer()->drawText(13, 6, firstLine);
    if (!secondLine.empty()) {
        getRenderer()->drawText(13, 7, secondLine);
    }
}

void MainMenuScreen::drawGlyphListMenu()
{
    if (currentMenu == &usbHostMenu) {
        refreshGlyphUsbHostMenuLabels();
    }

    drawGlyphBitmap(getRenderer(), bitmap_glyph_list_menu_base, 128, 64, 0, 0);

    std::string title = glyphListTitle(currentMenu, &profilesMenu, &modProfileMenu, &analogTriggersMenu, &lightShieldMenu, &modXAngleMenu, &modYAngleMenu, &modMagnitudeMenu, &modCoordinateMenu, &modSingleValueMenu, &inputModeMenu, &usbHostMenu, &backendSupportMenu, &dpadModeMenu, &socdModeMenu, &rgbBrightnessMenu, &turboModeMenu, &saveMenu);
    auto titleLines = glyphListTitleLines(title);
    if (titleLines.first.length() > 10) {
        titleLines.first.resize(10);
    }
    if (titleLines.second.length() > 10) {
        titleLines.second.resize(10);
    }
    getRenderer()->drawText(1, titleLines.second.empty() ? 3 : 2, titleLines.first);
    if (!titleLines.second.empty()) {
        getRenderer()->drawText(1, 3, titleLines.second);
    }

    if (currentMenu == &modCoordinateMenu) {
        getRenderer()->drawText(1, 2, "Magnitude");
        getRenderer()->drawText(1, 3, modCoordinateEditing ? "Editing" : "Select");
    }
    if (currentMenu == &modSingleValueMenu) {
        getRenderer()->drawText(1, 2, modSingleValueTitle);
        getRenderer()->drawText(1, 3, modSingleValueEditing ? "Editing" : "Select");
    }

    const uint16_t index = gpMenu->getIndex();
    const uint16_t menuSize = currentMenu->size();
    const uint16_t first = index > 1 ? index - 1 : 0;
    const uint16_t visible = 7;

    for (uint16_t line = 0; line < visible && (first + line) < menuSize; line++) {
        const uint16_t item = first + line;
        const bool selected = item == index;
        const std::string label = glyphMenuLabelText(currentMenu->at(item).label, selected, 8);

        const uint16_t y = line + 1;
        if (selected) {
            getRenderer()->drawText(11, y, ">");
        }
        getRenderer()->drawText(13, y, label);
        if (currentMenu->at(item).optionValue == currentMenu->at(item).currentValue()) {
            getRenderer()->drawText(20, y, "<");
        }
    }

}

void MainMenuScreen::drawGlyphControls()
{
    return;
}

bool MainMenuScreen::handleGlyphModProfileNavigation(GpioAction action)
{
    if (gpMenu == nullptr || currentMenu->empty()) {
        return false;
    }

    const int32_t selectedValue = currentMenu->at(gpMenu->getIndex()).optionValue;
    const bool coordinateEdit = currentMenu == &modCoordinateMenu &&
        (selectedValue == kGlyphModProfileActionCoordinateX || selectedValue == kGlyphModProfileActionCoordinateY);
    const bool shieldEdit = currentMenu == &modSingleValueMenu &&
        selectedValue == kGlyphModProfileActionSingleValue;

    if (!coordinateEdit && !shieldEdit) {
        return false;
    }

    if (action == GpioAction::MENU_NAVIGATION_SELECT) {
        if (coordinateEdit) {
            modCoordinateEditing = !modCoordinateEditing;
        } else {
            modSingleValueEditing = !modSingleValueEditing;
        }
        return true;
    }

    if (action == GpioAction::MENU_NAVIGATION_BACK) {
        if (coordinateEdit && modCoordinateEditing) {
            modCoordinateEditing = false;
            return true;
        }
        if (shieldEdit && modSingleValueEditing) {
            modSingleValueEditing = false;
            return true;
        }
        return false;
    }

    if (coordinateEdit && !modCoordinateEditing) {
        return false;
    }

    if (shieldEdit && !modSingleValueEditing) {
        return false;
    }

    int8_t delta = 0;
    if (action == GpioAction::MENU_NAVIGATION_LEFT || action == GpioAction::MENU_NAVIGATION_DOWN) {
        delta = -1;
    } else if (action == GpioAction::MENU_NAVIGATION_RIGHT || action == GpioAction::MENU_NAVIGATION_UP) {
        delta = 1;
    } else {
        return false;
    }

    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    if (coordinateEdit) {
        GlyphProfiles::adjustModProfileCoordinate(GlyphProfiles::modProfile(profile), modCoordinateMenuIsModX, modCoordinateMenuSlot, selectedValue == kGlyphModProfileActionCoordinateY, delta);
        populateGlyphModAngleMenus();
        populateGlyphModCoordinateMenu();
    } else {
        GlyphProfiles::adjustModProfileLightShield(GlyphProfiles::modProfile(profile), modSingleValueSource == kGlyphModProfileActionLightShield2, delta);
        populateGlyphLightShieldMenu();
        populateGlyphModSingleValueMenu();
    }

    GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
    gpMenu->setMenuData(currentMenu);
    if (gpMenu->getIndex() >= currentMenu->size()) {
        gpMenu->setIndex(currentMenu->size() - 1);
    }
    return true;
}
#endif

void MainMenuScreen::setMenu(std::vector<MenuEntry>* menu) {
    currentMenu = menu;
}

void MainMenuScreen::setMenuHome() {
    currentMenu = &mainMenu;
    previousMenu = nullptr;
    menuHistory.clear();

    exitToScreen = -1;
    prevValues = Storage::getInstance().GetGamepad()->debouncedGpio;
    isMenuReady = true;
}

int8_t MainMenuScreen::update() {
    if (isMenuReady) {
#ifndef GLYPH_DISPLAY_SCREEN
        GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
#endif
        Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
        uint16_t buttonState = getGamepad()->state.buttons;
        uint8_t dpadState = getGamepad()->state.dpad;

        if (prevValues != values) {
            if (values & mapMenuUp->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_UP);
            else if (values & mapMenuDown->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_DOWN);
            else if (values & mapMenuLeft->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_LEFT);
            else if (values & mapMenuRight->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_RIGHT);
            else if (values & mapMenuSelect->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_SELECT);
            else if (values & mapMenuBack->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_BACK);
            else if (values & mapMenuToggle->pinMask) {
                // Menu toggle will always exit out of main menu
#ifdef GLYPH_DISPLAY_SCREEN
                setGlyphButtonsScreenMode(false);
#endif
                exitToScreen = DisplayMode::BUTTONS;
                exitToScreenBeforePrompt = DisplayMode::BUTTONS;
            }
        }
#ifndef GLYPH_DISPLAY_SCREEN
        if (gamepadOptions.miniMenuGamepadInput == true ) {
            if (prevDpadState != dpadState ) {
                if (dpadState == mapMenuUp->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_UP);
                else if (dpadState == mapMenuDown->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_DOWN);
                else if (dpadState == mapMenuLeft->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_LEFT);
                else if (dpadState == mapMenuRight->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_RIGHT);
            }
            if ( prevButtonState != buttonState ) {
                if (buttonState == mapMenuSelect->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_SELECT);
                else if (buttonState == mapMenuBack->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_BACK);
            }
        }
#endif

        prevButtonState = buttonState;
        prevDpadState = dpadState;
        prevValues = values;

        // Core0 Event Navigations
        if (eventAction != GpioAction::NONE) {
            updateMenuNavigation(eventAction);
            eventAction = GpioAction::NONE;
        }

        if ((exitToScreen != -1) && ((changeRequiresSave) || (changeRequiresReboot))) {
            // trying to exit menu but a change requires a save/reboot
            exitToScreenBeforePrompt = exitToScreen;
            exitToScreen = -1;
            screenIsPrompting = true;
        }

        return exitToScreen;
    } else {
        return -1;
    }
}

void MainMenuScreen::updateEventMenuNavigation(GpioAction action) {
    eventAction = action;
}

void MainMenuScreen::updateMenuNavigation(GpioAction action) {
    if (gpMenu == nullptr)
        return;

    uint16_t menuIndex = gpMenu->getIndex();
    uint16_t menuSize = gpMenu->getDataSize();

    if (isMenuReady) {
        if (screenIsPrompting) {
            switch(action) {
                case GpioAction::MENU_NAVIGATION_UP:
                case GpioAction::MENU_NAVIGATION_DOWN:
                case GpioAction::MENU_NAVIGATION_LEFT:
                case GpioAction::MENU_NAVIGATION_RIGHT:
                    promptChoice = !promptChoice;
                    break;
                case GpioAction::MENU_NAVIGATION_SELECT:
                    if (promptChoice) {
                        saveOptions();
                    } else {
                        resetOptions();
#ifdef GLYPH_DISPLAY_SCREEN
                        setGlyphButtonsScreenMode(false);
#endif
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_BACK:
                    // back again goes back to the menu
                    screenIsPrompting = false;
                    break;
                default:
                    break;
            }
        } else {
#ifdef GLYPH_DISPLAY_SCREEN
            if (menuSize == 0) {
                return;
            }

            if (handleGlyphModProfileNavigation(action)) {
                return;
            }

            auto prepareGlyphSubmenu = [&](std::vector<MenuEntry>* menu, uint16_t index) {
                if (menu == nullptr || menu->empty() || index >= menu->size()) {
                    return;
                }

                const MenuEntry& entry = menu->at(index);
                if (entry.submenu == nullptr) {
                    return;
                }

                if (menu == &modXAngleMenu || menu == &modYAngleMenu) {
                    if (entry.submenu == &modMagnitudeMenu) {
                        modMagnitudeMenuIsModX = (menu == &modXAngleMenu);
                        populateGlyphModAngleMenus();
                        return;
                    }

                    if (entry.submenu == &modCoordinateMenu) {
                        modCoordinateMenuIsModX = (menu == &modXAngleMenu);
                        const int32_t actionValue = entry.optionValue;
                        modCoordinateMenuSlot = (actionValue == kGlyphModProfileActionEditModXCRight || actionValue == kGlyphModProfileActionEditModYCRight) ? 0 :
                            (actionValue == kGlyphModProfileActionEditModXCUp || actionValue == kGlyphModProfileActionEditModYCUp) ? 1 :
                            (actionValue == kGlyphModProfileActionEditModXCLeft || actionValue == kGlyphModProfileActionEditModYCLeft) ? 2 : 3;
                        modCoordinateEditing = false;
                        populateGlyphModCoordinateMenu();
                        return;
                    }
                }

                if (menu == &modMagnitudeMenu && entry.submenu == &modCoordinateMenu) {
                    modCoordinateMenuIsModX = modMagnitudeMenuIsModX;
                    modCoordinateMenuSlot = entry.optionValue == (modMagnitudeMenuIsModX ? kGlyphModProfileActionEditModXBase : kGlyphModProfileActionEditModYBase) ? 4 : 5;
                    modCoordinateEditing = false;
                    populateGlyphModCoordinateMenu();
                    return;
                }

                if (menu == &lightShieldMenu && entry.submenu == &modSingleValueMenu) {
                    modSingleValueSource = entry.optionValue;
                    modSingleValueTitle = entry.label;
                    modSingleValueEditing = false;
                    populateGlyphModSingleValueMenu();
                }
            };

            if (currentMenu == &mainMenu) {
                switch (action) {
                    case GpioAction::MENU_NAVIGATION_UP:
                    case GpioAction::MENU_NAVIGATION_LEFT:
                        gpMenu->setIndex(menuIndex == 0 ? menuSize - 1 : menuIndex - 1);
                        break;
                    case GpioAction::MENU_NAVIGATION_DOWN:
                    case GpioAction::MENU_NAVIGATION_RIGHT:
                        gpMenu->setIndex((menuIndex + 1) % menuSize);
                        break;
                    case GpioAction::MENU_NAVIGATION_SELECT:
                        if (currentMenu->at(menuIndex).submenu != nullptr) {
                            prepareGlyphSubmenu(currentMenu, menuIndex);
                            menuHistory.push_back(currentMenu);
                            previousMenu = currentMenu;
                            currentMenu = currentMenu->at(menuIndex).submenu;
                            gpMenu->setMenuData(currentMenu);
                            gpMenu->setMenuTitle(previousMenu->at(menuIndex).label);
                            gpMenu->setIndex(0);
                        } else {
                            currentMenu->at(menuIndex).action();
                        }
                        break;
                    case GpioAction::MENU_NAVIGATION_BACK:
#ifdef GLYPH_DISPLAY_SCREEN
                        setGlyphButtonsScreenMode(false);
#endif
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                        break;
                    default:
                        break;
                }
                return;
            }

            switch (action) {
                case GpioAction::MENU_NAVIGATION_UP:
                case GpioAction::MENU_NAVIGATION_LEFT:
                    if (menuIndex > 0) {
                        gpMenu->setIndex(menuIndex - 1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_DOWN:
                case GpioAction::MENU_NAVIGATION_RIGHT:
                    if (menuIndex < menuSize - 1) {
                        gpMenu->setIndex(menuIndex + 1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_SELECT:
                    if (currentMenu->at(menuIndex).submenu != nullptr) {
                        prepareGlyphSubmenu(currentMenu, menuIndex);
                        menuHistory.push_back(currentMenu);
                        previousMenu = currentMenu;
                        currentMenu = currentMenu->at(menuIndex).submenu;
                        gpMenu->setMenuData(currentMenu);
                        gpMenu->setMenuTitle(previousMenu->at(menuIndex).label);
                        gpMenu->setIndex(0);
                    } else {
                        currentMenu->at(menuIndex).action();
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_BACK:
                    if (!menuHistory.empty()) {
                        modCoordinateEditing = false;
                        modSingleValueEditing = false;
                        currentMenu = menuHistory.back();
                        menuHistory.pop_back();
                        previousMenu = menuHistory.empty() ? nullptr : menuHistory.back();
                        gpMenu->setMenuData(currentMenu);
                        gpMenu->setMenuTitle(MAIN_MENU_NAME);
                        gpMenu->setIndex(0);
                    } else {
#ifdef GLYPH_DISPLAY_SCREEN
                        setGlyphButtonsScreenMode(false);
#endif
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    }
                    break;
                default:
                    break;
            }
            return;
#endif
            switch (action) { 
                case GpioAction::MENU_NAVIGATION_UP:
                    if ( menuIndex > 0 ) {
                        gpMenu->setIndex(menuIndex-1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_DOWN:
                    if (menuIndex < menuSize-1) {
                        gpMenu->setIndex(menuIndex+1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_LEFT:
                    if ((menuIndex-menuLineSize) > 0) {
                        gpMenu->setIndex(menuIndex - menuLineSize);
                    } else {
                        gpMenu->setIndex(0);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_RIGHT:
                    if ((menuIndex+menuLineSize) < (menuSize-1)) {
                        gpMenu->setIndex(menuIndex + menuLineSize);
                    } else {
                        gpMenu->setIndex(menuSize-1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_SELECT:
                    if (currentMenu->at(menuIndex).submenu != nullptr) {
                        previousMenu = currentMenu;
                        currentMenu = currentMenu->at(menuIndex).submenu;
                        gpMenu->setMenuData(currentMenu);
                        gpMenu->setMenuTitle(previousMenu->at(menuIndex).label);
                        gpMenu->setIndex(0);
                    } else {
                        currentMenu->at(menuIndex).action();
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_BACK:
                    if (previousMenu != nullptr) {
                        currentMenu = previousMenu;
                        previousMenu = nullptr;
                        gpMenu->setMenuData(currentMenu);
                        gpMenu->setMenuTitle(MAIN_MENU_NAME);
                        gpMenu->setIndex(0);
                    } else {
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void MainMenuScreen::chooseAndReturn() {
    if (!menuHistory.empty()) {
        modCoordinateEditing = false;
        modSingleValueEditing = false;
        currentMenu = menuHistory.back();
        menuHistory.pop_back();
        previousMenu = menuHistory.empty() ? nullptr : menuHistory.back();
        gpMenu->setMenuData(currentMenu);
        gpMenu->setMenuTitle(MAIN_MENU_NAME);
        gpMenu->setIndex(0);
    } else {
#ifdef GLYPH_DISPLAY_SCREEN
        setGlyphButtonsScreenMode(false);
#endif
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        isPressed = false;
    }
}

void MainMenuScreen::saveAndExit() {
    saveOptions();
#ifdef GLYPH_DISPLAY_SCREEN
    setGlyphButtonsScreenMode(false);
#endif
    exitToScreen = DisplayMode::BUTTONS;
}

void MainMenuScreen::exitOnly() {
#ifdef GLYPH_DISPLAY_SCREEN
    setGlyphButtonsScreenMode(false);
#endif
    exitToScreen = DisplayMode::BUTTONS;
}

void MainMenuScreen::openInputViewer() {
#ifdef GLYPH_DISPLAY_SCREEN
    setGlyphButtonsScreenMode(true);
#endif
    exitToScreen = DisplayMode::BUTTONS;
    exitToScreenBeforePrompt = DisplayMode::BUTTONS;
}

void MainMenuScreen::rebootBootsel() {
    EventManager::getInstance().triggerEvent(new GPRestartEvent(System::BootMode::USB));
    exitToScreen = DisplayMode::RESTART;
}

void MainMenuScreen::rebootGlyphConfig() {
    EventManager::getInstance().triggerEvent(new GPRestartEvent(System::BootMode::GLYPH_CONFIG));
    exitToScreen = DisplayMode::RESTART;
}

int32_t MainMenuScreen::modeValue() {
    return -1;
}

void MainMenuScreen::selectInputMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        InputMode valueToSave = (InputMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
        updateInputMode = valueToSave;

#ifdef GLYPH_DISPLAY_SCREEN
        if (prevInputMode != valueToSave) {
            Storage::getInstance().getGamepadOptions().inputMode = valueToSave;
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, true));
            exitToScreen = DisplayMode::RESTART;
        } else {
            chooseAndReturn();
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
        screenIsPrompting = false;
        return;
#endif

        chooseAndReturn();

        if (prevInputMode != valueToSave) {
            // input mode requires a save and reboot
            changeRequiresReboot = true;
            changeRequiresSave = true;
        }
    }
}

int32_t MainMenuScreen::currentInputMode() {
    return updateInputMode;
}

void MainMenuScreen::selectDPadMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        DpadMode valueToSave = (DpadMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
        updateDpadMode = valueToSave;

#ifdef GLYPH_DISPLAY_SCREEN
        if (prevDpadMode != valueToSave) {
            Storage::getInstance().getGamepadOptions().dpadMode = valueToSave;
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
        screenIsPrompting = false;
        setGlyphButtonsScreenMode(false);
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        return;
#endif

        chooseAndReturn();

        if (prevDpadMode != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentDpadMode() {
    return updateDpadMode;
}

void MainMenuScreen::selectSOCDMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        SOCDMode valueToSave = (SOCDMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
        updateSocdMode = valueToSave;

#ifdef GLYPH_DISPLAY_SCREEN
        const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
        if (GlyphProfiles::socdMode(profile) != valueToSave) {
            GlyphProfiles::setSOCDMode(profile, valueToSave);
            GlyphProfiles::writeToConfig(Storage::getInstance().getGlyphOptions());
            Storage::getInstance().getGamepadOptions().socdMode = valueToSave;
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
        screenIsPrompting = false;
        setGlyphButtonsScreenMode(false);
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        return;
#endif

        chooseAndReturn();

        if (prevSocdMode != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentSOCDMode() {
#ifdef GLYPH_DISPLAY_SCREEN
    const uint8_t profile = updateProfile >= 1 ? updateProfile : Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    return GlyphProfiles::socdMode(profile);
#endif
    return updateSocdMode;
}

void MainMenuScreen::resetOptions() {
    if (changeRequiresSave) {
        if (prevInputMode != updateInputMode) updateInputMode = prevInputMode;
        if (prevDpadMode != updateDpadMode) updateDpadMode = prevDpadMode;
        if (prevSocdMode != updateSocdMode) updateSocdMode = prevSocdMode;
        if (prevProfile != updateProfile) updateProfile = prevProfile;
        if (prevFocus != updateFocus) updateFocus = prevFocus;
        if (prevTurbo != updateTurbo) updateTurbo = prevTurbo;
    }

    changeRequiresSave = false;
    changeRequiresReboot = false;
    screenIsPrompting = false;
}

void MainMenuScreen::saveOptions() {
    GamepadOptions& options = Storage::getInstance().getGamepadOptions();
    AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();

    if (changeRequiresSave) {
        bool saveHasChanged = false;
        if (prevInputMode != updateInputMode) {
            options.inputMode = updateInputMode;
            saveHasChanged = true;
        }
        if (prevDpadMode != updateDpadMode) {
            options.dpadMode = updateDpadMode;
            saveHasChanged = true;
        }
        if (prevSocdMode != updateSocdMode) {
            options.socdMode = updateSocdMode;
            saveHasChanged = true;
        }
        if (prevProfile != updateProfile) {
            options.profileNumber = updateProfile;
            saveHasChanged = true;
        }
        if ((bool)prevFocus != (bool)updateFocus) {
            addonOptions.focusModeOptions.enabled = (bool)updateFocus;
            saveHasChanged = true;
        }
        if ((bool)prevTurbo != (bool)updateTurbo) {
            addonOptions.turboOptions.enabled = (bool)updateTurbo;
            saveHasChanged = true;
        }

        if (saveHasChanged) {
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, changeRequiresReboot));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
    }

    screenIsPrompting = false;

    if (exitToScreenBeforePrompt != -1) {
#ifdef GLYPH_DISPLAY_SCREEN
        if (exitToScreenBeforePrompt == DisplayMode::BUTTONS) {
            setGlyphButtonsScreenMode(false);
        }
#endif
        exitToScreen = exitToScreenBeforePrompt;
        exitToScreenBeforePrompt = -1;
    }
}

void MainMenuScreen::selectProfile() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        uint8_t valueToSave = currentMenu->at(gpMenu->getIndex()).optionValue;
        prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
        updateProfile = valueToSave;

#ifdef GLYPH_DISPLAY_SCREEN
        Storage::getInstance().getGamepadOptions().socdMode = GlyphProfiles::socdMode(valueToSave);
        populateGlyphBackendMenu();
        populateGlyphBackendSupportMenu();
        populateGlyphModProfileMenu();
        if (prevProfile != valueToSave) {
            Storage::getInstance().getGamepadOptions().profileNumber = valueToSave;
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
        screenIsPrompting = false;
        setGlyphButtonsScreenMode(false);
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        return;
#endif

        chooseAndReturn();

        if (prevProfile != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentProfile() {
    return updateProfile;
}

void MainMenuScreen::selectFocusMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        bool valueToSave = (bool)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
        updateFocus = valueToSave;

        chooseAndReturn();

        if (prevFocus != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentFocusMode() {
    return updateFocus;
}

void MainMenuScreen::selectTurboMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        bool valueToSave = (bool)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;
        updateTurbo = valueToSave;

#ifdef GLYPH_DISPLAY_SCREEN
        if (prevTurbo != valueToSave) {
            Storage::getInstance().getAddonOptions().turboOptions.enabled = valueToSave;
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
        screenIsPrompting = false;
        setGlyphButtonsScreenMode(false);
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        return;
#endif

        chooseAndReturn();

        if (prevTurbo != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentTurboMode() {
    return updateTurbo;
}

void MainMenuScreen::selectRgbBrightness() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue == -1) {
        return;
    }

    AnimationOptions& animationOptions = Storage::getInstance().getAnimationOptions();
    const uint8_t valueToSave = static_cast<uint8_t>(currentMenu->at(gpMenu->getIndex()).optionValue);
    if (animationOptions.brightness != valueToSave) {
        animationOptions.brightness = valueToSave;
        EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
    }

#ifdef GLYPH_DISPLAY_SCREEN
    changeRequiresSave = false;
    changeRequiresReboot = false;
    screenIsPrompting = false;
    setGlyphButtonsScreenMode(false);
    exitToScreen = DisplayMode::BUTTONS;
    exitToScreenBeforePrompt = DisplayMode::BUTTONS;
#else
    chooseAndReturn();
#endif
}

int32_t MainMenuScreen::currentRgbBrightness() {
    return Storage::getInstance().getAnimationOptions().brightness;
}
