#ifndef _GAMECUBE_DRIVER_H_
#define _GAMECUBE_DRIVER_H_

#include "drivers/hid/HIDDriver.h"

class GameCubeDriver : public HIDDriver {
public:
    void initialize() override;
    bool process(Gamepad *gamepad) override;
};

#endif
