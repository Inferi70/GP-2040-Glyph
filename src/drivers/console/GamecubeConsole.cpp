#include "drivers/console/GamecubeConsole.hpp"

#include <hardware/timer.h>
#include <pico/time.h>

GamecubeConsole::GamecubeConsole(uint pin, PIO pio, int sm, int offset) {
    joybus_port_init(&port, pin, pio, sm, offset);
}

GamecubeConsole::~GamecubeConsole() {
    joybus_port_terminate(&port);
}

bool __no_inline_not_in_flash_func(GamecubeConsole::Detect)() {
    uint8_t received[1];

    for (uint8_t attempts = 0; attempts < 10; attempts++) {
        if (joybus_receive_bytes(&port, received, 1, 10000, true) != 1) {
            continue;
        }

        switch ((GamecubeCommand)received[0]) {
            case GamecubeCommand::PROBE:
            case GamecubeCommand::RESET:
                busy_wait_us(reply_delay);
                joybus_send_bytes(&port, (uint8_t *)&default_gc_status, sizeof(gc_status_t));
                break;
            case GamecubeCommand::ORIGIN:
            case GamecubeCommand::RECALIBRATE:
                return true;
            case GamecubeCommand::POLL:
                WaitForPollEnd();
                return true;
            default:
                busy_wait_us(reset_wait_period_us);
                joybus_port_reset(&port);
        }
    }

    return false;
}

bool __no_inline_not_in_flash_func(GamecubeConsole::WaitForPoll)() {
    while (true) {
        WaitForPollStart();
        PollStatus status = WaitForPollEnd();

        if (status == PollStatus::ERROR) {
            busy_wait_us(reset_wait_period_us);
            joybus_port_reset(&port);
            continue;
        }

        return status == PollStatus::RUMBLE_ON;
    }
}

bool __no_inline_not_in_flash_func(GamecubeConsole::WaitForPollStart)() {
    uint8_t received[1];
    joybus_receive_bytes(&port, received, 1, receive_timeout_us, false);

    switch ((GamecubeCommand)received[0]) {
        case GamecubeCommand::RESET:
        case GamecubeCommand::PROBE:
            busy_wait_us(reply_delay);
            joybus_send_bytes(&port, (uint8_t *)&default_gc_status, sizeof(gc_status_t));
            break;
        case GamecubeCommand::RECALIBRATE:
        case GamecubeCommand::ORIGIN:
            busy_wait_us(reply_delay);
            joybus_send_bytes(&port, (uint8_t *)&default_gc_origin, sizeof(gc_origin_t));
            break;
        case GamecubeCommand::POLL:
            return false;
        default:
            busy_wait_us(reset_wait_period_us);
            joybus_port_reset(&port);
    }

    return true;
}

PollStatus __no_inline_not_in_flash_func(GamecubeConsole::WaitForPollEnd)() {
    uint8_t received[2];
    uint received_len = joybus_receive_bytes(&port, received, 2, receive_timeout_us, true);

    if (received_len != 2 || received[0] > 0x07) {
        return PollStatus::ERROR;
    }

    readingMode = received[0];
    receiveEnd = make_timeout_time_us(reply_delay);
    return (received[1] & 0x01) ? PollStatus::RUMBLE_ON : PollStatus::RUMBLE_OFF;
}

void __no_inline_not_in_flash_func(GamecubeConsole::SendReport)(gc_report_t *report) {
    while (!time_reached(receiveEnd)) {
        tight_loop_contents();
    }

    joybus_send_bytes(&port, (uint8_t *)report, sizeof(gc_report_t));
}

int GamecubeConsole::GetOffset() {
    return port.offset;
}
