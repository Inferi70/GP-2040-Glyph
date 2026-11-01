#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <cstdint>

namespace System {
    // Returns the size of on-board flash memory reserved by the config
    uint32_t getTotalFlash();
    // Returns the amount of on-board flash memory used by the firmware in bytes
    uint32_t getUsedFlash();
    // Returns the amount of physical flash memory on the board
    uint32_t getPhysicalFlash();
    // Returns the amount of memory used for static allocations in bytes
    uint32_t getStaticAllocs();
    // Returns the total size of heap memory in bytes
    uint32_t getTotalHeap();
    // Returns the about of heap memory currently allocated in bytes
    uint32_t getUsedHeap();

    enum class BootMode : uint32_t {
        DEFAULT = 0,
        GAMEPAD = 0x43d566cd,
        WEBCONFIG = 0xe77784a5,
        GLYPH_CONFIG = 0x676c7970,
        USB = 0xf737e4e1,
        RESCUE_XINPUT = 0x78494e50,
        INPUT_MODE_XINPUT = 0x78496d31,
        INPUT_MODE_XBONE = 0x78626f31,
        INPUT_MODE_GENERIC = 0x67656e31,
        INPUT_MODE_SWITCH = 0x73776831,
        INPUT_MODE_SWITCH_PRO = 0x73777031,
        INPUT_MODE_PS4 = 0x70733431,
        INPUT_MODE_PS5 = 0x70733531,
        INPUT_MODE_P5GENERAL = 0x70356731,
        INPUT_MODE_GAMECUBE = 0x67633131,
    };

    // Reboots the device and places the supplied BootMode value in a watchdog scratch register
    // This value can be retrieved after reboot by client code, which can then take the required actions
    void reboot(BootMode bootMode);
    // Retrieves the BootMode value from the watchdog scratch register and resets its value to BootMode::DEFAULT
    BootMode takeBootMode();
}

#endif
