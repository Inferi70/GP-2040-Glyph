#ifndef _LEGACY_INPUTMODE_HPP_
#define _LEGACY_INPUTMODE_HPP_

#include "drivers/legacy/state.hpp"

class LegacyInputMode {
public:
    virtual ~LegacyInputMode() {}
    virtual void UpdateOutputs(
        const InputState &inputs,
        OutputState &outputs,
        CommunicationBackendId backend_id
    ) = 0;
};

#endif
