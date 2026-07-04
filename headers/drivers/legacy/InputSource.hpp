#ifndef _LEGACY_INPUTSOURCE_HPP_
#define _LEGACY_INPUTSOURCE_HPP_

#include "drivers/legacy/state.hpp"

enum class InputScanSpeed {
    SLOW,
    MEDIUM,
    FAST,
};

class InputSource {
public:
    virtual ~InputSource() {}
    virtual InputScanSpeed ScanSpeed() = 0;
    virtual void UpdateInputs(InputState &inputs) = 0;
};

#endif
