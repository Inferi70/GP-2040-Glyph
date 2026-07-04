#ifndef _JOYBUS_GAMECUBECONSOLE_HPP
#define _JOYBUS_GAMECUBECONSOLE_HPP

#include "drivers/console/gamecube_definitions.h"
#include "drivers/console/joybus.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

enum class PollStatus {
    RUMBLE_OFF,
    RUMBLE_ON,
    ERROR,
};

class GamecubeConsole {
public:
    GamecubeConsole(uint pin, PIO pio = pio0, int sm = -1, int offset = -1);
    ~GamecubeConsole();

    bool Detect();
    bool WaitForPoll();
    bool WaitForPollStart();
    PollStatus WaitForPollEnd();
    void SendReport(gc_report_t *report);
    int GetOffset();

private:
    static constexpr uint incoming_bit_length_us = 5;
    static constexpr uint max_command_bytes = 3;
    static constexpr uint receive_timeout_us = incoming_bit_length_us * 10;
    static constexpr uint reset_wait_period_us =
        (incoming_bit_length_us * 8) * (max_command_bytes - 1) + receive_timeout_us;
    static constexpr uint64_t reply_delay = incoming_bit_length_us - 1;

    joybus_port_t port;
    absolute_time_t receiveEnd;
    uint8_t readingMode = 3;
};

#endif
