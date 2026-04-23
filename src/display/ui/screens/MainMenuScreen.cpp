#include "MainMenuScreen.h"
#include "glyph/assets/GlyphButtonBitmaps.h"
#include "glyph/assets/GlyphMenuBitmaps.h"
#include "glyph/glyph_profiles.h"
#include "display/ui/screens/GlyphInputScreen.h"
#include "hardware/watchdog.h"
#include "system.h"

#include <utility>

extern uint32_t getMillis();

#ifdef GLYPH_DISPLAY_SCREEN
namespace
{
std::pair<std::string, std::string> glyphTopLabelLines(const std::string& label)
{
    if (label == "Input Viewer") return {"Input", "Viewer"};
    if (label == "Input Mode" || label == "USB Mode") return {"USB", "Mode"};
    if (label == "USB Host") return {"USB", "Host"};
    if (label == "USB Support") return {"USB", "Support"};
    if (label == "Profiles") return {"Profile", ""};
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

void setGlyphButtonsScreenMode(bool inputViewer)
{
    GlyphInputScreen::setInputViewerMode(inputViewer);
}

std::string glyphListTitle(std::vector<MenuEntry>* menu,
                           std::vector<MenuEntry>* profilesMenu,
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
}
#endif

void MainMenuScreen::init() {
    getRenderer()->clearScreen();
    currentMenu = &mainMenu;
    previousMenu = nullptr;

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
        MenuEntry menuEntry = {GlyphProfiles::name(profileCtr), NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr};
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
        {"XInput", GlyphProfiles::BackendXInput},
        {"DInput", GlyphProfiles::BackendDInput},
        {"Switch", GlyphProfiles::BackendSwitch},
        {"PS4", GlyphProfiles::BackendPS4},
        {"PS5", GlyphProfiles::BackendPS5},
    };

    for (const auto& backend : backends) {
        backendSupportMenu.push_back({backend.label, NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphBackendSupport, this), std::bind(&MainMenuScreen::toggleGlyphBackendSupport, this), backend.mask});
    }
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

void MainMenuScreen::refreshGlyphUsbHostMenuLabels()
{
    const bool storedPortEnabled = Storage::getInstance().getPeripheralOptions().blockUSB0.enabled;

    usbHostMenu.clear();
    if (storedPortEnabled) {
        usbHostMenu.push_back({"Port Disable", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), 32});
    }
    usbHostMenu.push_back({"XInput Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), 16});
    usbHostMenu.push_back({"PS4 Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), 4});
    usbHostMenu.push_back({"PS5 Auth", NULL, nullptr, std::bind(&MainMenuScreen::currentGlyphUsbHostOption, this), std::bind(&MainMenuScreen::toggleGlyphUsbHostOption, this), 8});

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
    bool changed = false;
    auto hasUsbHostConsumer = [&]() {
        return addonOptions.gamepadUSBHostOptions.enabled ||
               gamepadOptions.xinputAuthType == INPUT_MODE_AUTH_TYPE_USB ||
               gamepadOptions.ps4AuthType == INPUT_MODE_AUTH_TYPE_USB ||
               gamepadOptions.ps5AuthType == INPUT_MODE_AUTH_TYPE_USB;
    };

    switch (option) {
        case 4:
            if (gamepadOptions.ps4AuthType != INPUT_MODE_AUTH_TYPE_USB) {
                peripheralOptions.blockUSB0.enabled = true;
            }
            gamepadOptions.ps4AuthType = gamepadOptions.ps4AuthType == INPUT_MODE_AUTH_TYPE_USB ?
                INPUT_MODE_AUTH_TYPE_NONE : INPUT_MODE_AUTH_TYPE_USB;
            changed = true;
            break;
        case 8:
            if (gamepadOptions.ps5AuthType != INPUT_MODE_AUTH_TYPE_USB) {
                peripheralOptions.blockUSB0.enabled = true;
            }
            gamepadOptions.ps5AuthType = gamepadOptions.ps5AuthType == INPUT_MODE_AUTH_TYPE_USB ?
                INPUT_MODE_AUTH_TYPE_NONE : INPUT_MODE_AUTH_TYPE_USB;
            changed = true;
            break;
        case 16:
            if (gamepadOptions.xinputAuthType != INPUT_MODE_AUTH_TYPE_USB) {
                peripheralOptions.blockUSB0.enabled = true;
            }
            gamepadOptions.xinputAuthType = gamepadOptions.xinputAuthType == INPUT_MODE_AUTH_TYPE_USB ?
                INPUT_MODE_AUTH_TYPE_NONE : INPUT_MODE_AUTH_TYPE_USB;
            changed = true;
            break;
        case 32:
            peripheralOptions.blockUSB0.enabled = false;
            addonOptions.gamepadUSBHostOptions.enabled = false;
            addonOptions.keyboardHostOptions.enabled = false;
            gamepadOptions.xinputAuthType = INPUT_MODE_AUTH_TYPE_NONE;
            gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_NONE;
            gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_NONE;
            changed = true;
            break;
        default:
            break;
    }

    if (changed) {
        if (!hasUsbHostConsumer()) {
            peripheralOptions.blockUSB0.enabled = false;
        }
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

    switch (option) {
        case 4:
            return gamepadOptions.ps4AuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case 8:
            return gamepadOptions.ps5AuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case 16:
            return gamepadOptions.xinputAuthType == INPUT_MODE_AUTH_TYPE_USB ? option : 0;
        case 32:
            return peripheralOptions.blockUSB0.enabled ? option : 0;
        default:
            return 0;
    }
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

    constexpr uint16_t visibleIcons = 4;
    uint16_t firstIcon = 0;
    if (menuSize > visibleIcons) {
        firstIcon = index > 1 ? index - 1 : 0;
        if (firstIcon + visibleIcons > menuSize) {
            firstIcon = menuSize - visibleIcons;
        }
    }

    uint16_t x = 2;
    for (uint16_t i = firstIcon; i < menuSize && i < firstIcon + visibleIcons; i++) {
        const bool selected = i == index;
        const uint16_t size = selected ? 28 : 10;
        const uint16_t y = selected ? 8 : 17;

        if (selected) {
            drawGlyphBitmap(getRenderer(), glyphLargeIcon(currentMenu->at(i).label), 28, 28, x, y);
        } else {
            drawGlyphBitmap(getRenderer(), glyphSmallIcon(currentMenu->at(i).label), 10, 10, x, y);
        }
        x += size + 2;
    }

    auto labelLines = glyphTopLabelLines(currentMenu->at(index).label);
    std::string firstLine = labelLines.first;
    std::string secondLine = labelLines.second;
    if (firstLine.length() > 8) firstLine.resize(8);
    if (secondLine.length() > 8) secondLine.resize(8);

    getRenderer()->drawText(12, 5, firstLine);
    if (!secondLine.empty()) {
        getRenderer()->drawText(12, 6, secondLine);
    }
}

void MainMenuScreen::drawGlyphListMenu()
{
    if (currentMenu == &usbHostMenu) {
        refreshGlyphUsbHostMenuLabels();
    }

    drawGlyphBitmap(getRenderer(), bitmap_glyph_list_menu_base, 128, 64, 0, 0);

    std::string title = glyphListTitle(currentMenu, &profilesMenu, &inputModeMenu, &usbHostMenu, &backendSupportMenu, &dpadModeMenu, &socdModeMenu, &rgbBrightnessMenu, &turboModeMenu, &saveMenu);
    if (title.length() > 14) {
        title.resize(14);
    }
    getRenderer()->drawText(2, 0, title);

    const uint16_t index = gpMenu->getIndex();
    const uint16_t menuSize = currentMenu->size();
    const uint16_t first = index > 1 ? index - 1 : 0;
    const uint16_t visible = 5;

    for (uint16_t line = 0; line < visible && (first + line) < menuSize; line++) {
        const uint16_t item = first + line;
        const bool selected = item == index;
        std::string label = currentMenu->at(item).label;
        if (label.length() > 14) {
            label.resize(14);
        }

        const uint16_t y = line + 1;
        if (selected) {
            getRenderer()->drawText(2, y, ">");
        }
        getRenderer()->drawText(4, y, label);
        if (currentMenu->at(item).optionValue == currentMenu->at(item).currentValue()) {
            getRenderer()->drawText(20, y, "<");
        }
    }

    if (currentMenu == &profilesMenu && menuSize > 0) {
        const uint8_t profile = currentMenu->at(index).optionValue;
        std::string backend = GlyphProfiles::backendSummary(profile);
        if (backend.length() > 12) backend.resize(12);
        getRenderer()->drawText(2, 6, backend);
    }
}

void MainMenuScreen::drawGlyphControls()
{
    const bool topLevel = currentMenu == &mainMenu;
    const unsigned char* icons[4] = {
        Back12,
        topLevel ? LeftArrow12 : UpArrow12,
        topLevel ? RightArrow12 : DownArrow12,
        Confirm12,
    };
    for (uint8_t i = 0; i < 4; i++) {
        const uint16_t x = 2 + (i * 18);
        drawGlyphBitmap(getRenderer(), icons[i], 12, 12, x + 2, 48);
    }
}
#endif

void MainMenuScreen::setMenu(std::vector<MenuEntry>* menu) {
    currentMenu = menu;
}

void MainMenuScreen::setMenuHome() {
    currentMenu = &mainMenu;
    previousMenu = nullptr;

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
    if (previousMenu != nullptr) {
        currentMenu = previousMenu;
        previousMenu = nullptr;
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
