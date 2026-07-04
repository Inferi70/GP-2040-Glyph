#ifndef _LEGACY_GAMEPADINPUTSOURCE_HPP_
#define _LEGACY_GAMEPADINPUTSOURCE_HPP_

#include "drivers/legacy/InputSource.hpp"
#include "gamepad.h"

class GamepadInputSource : public InputSource {
public:
    explicit GamepadInputSource(InputScanSpeed scan_speed) : scanSpeed(scan_speed) {}

    void setGamepad(Gamepad *gamepad) { currentGamepad = gamepad; }
    InputScanSpeed ScanSpeed() override { return scanSpeed; }
    void UpdateInputs(InputState &inputs) override;

private:
    InputScanSpeed scanSpeed;
    Gamepad *currentGamepad = nullptr;
};

#endif
