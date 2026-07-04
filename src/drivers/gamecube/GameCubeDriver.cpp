#include "drivers/gamecube/GameCubeDriver.h"

void GameCubeDriver::initialize() {
    HIDDriver::initialize();
}

bool GameCubeDriver::process(Gamepad *gamepad) {
    (void)gamepad;
    return false;
}
