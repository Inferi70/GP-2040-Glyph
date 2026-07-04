#include "drivers/legacy/GamecubeBackend.hpp"

#include <pico/multicore.h>

GamecubeBackend::GamecubeBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count,
    uint data_pin,
    PIO pio,
    int sm,
    int offset
)
    : CommunicationBackend(inputs, input_sources, input_source_count),
      _gamecube(data_pin, pio, sm, offset) {
    _report = default_gc_report;
}

CommunicationBackendId GamecubeBackend::BackendId() {
    return COMMS_BACKEND_GAMECUBE;
}

bool GamecubeBackend::Detect() {
    return _gamecube.Detect();
}

void __no_inline_not_in_flash_func(GamecubeBackend::SendReport)() {
    ScanInputs(InputScanSpeed::SLOW);
    ScanInputs(InputScanSpeed::MEDIUM);

    bool pollTime = true;
    while (pollTime == true) {
        ScanInputs(InputScanSpeed::FAST);
        pollTime = _gamecube.WaitForPollStart();
    }

    multicore_lockout_start_blocking();
    UpdateOutputs();

    _report.a = _outputs.a;
    _report.b = _outputs.b;
    _report.x = _outputs.x;
    _report.y = _outputs.y;
    _report.z = _outputs.buttonR;
    _report.l = _outputs.triggerLDigital;
    _report.r = _outputs.triggerRDigital;
    _report.start = _outputs.start;
    _report.dpad_left = _outputs.dpadLeft;
    _report.dpad_right = _outputs.dpadRight;
    _report.dpad_down = _outputs.dpadDown;
    _report.dpad_up = _outputs.dpadUp;

    _report.stick_x = _outputs.leftStickX;
    _report.stick_y = _outputs.leftStickY;
    _report.cstick_x = _outputs.rightStickX;
    _report.cstick_y = _outputs.rightStickY;
    _report.l_analog = _outputs.triggerLAnalog;
    _report.r_analog = _outputs.triggerRAnalog;

    if (_gamecube.WaitForPollEnd() != PollStatus::ERROR) {
        _gamecube.SendReport(&_report);
    }
    multicore_lockout_end_blocking();
}
