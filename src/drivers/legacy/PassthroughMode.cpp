#include "drivers/legacy/PassthroughMode.hpp"

#include <cstddef>

void PassthroughMode::UpdateOutputs(
    const InputState &inputs,
    OutputState &outputs,
    CommunicationBackendId
) {
    outputs.buttons = inputs.mappedButtons;
    for (size_t i = 0; i < 6; i++) {
        outputs.analog_axes[i] = inputs.analog_axes[i];
    }
}
