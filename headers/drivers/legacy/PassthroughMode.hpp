#ifndef _LEGACY_PASSTHROUGHMODE_HPP_
#define _LEGACY_PASSTHROUGHMODE_HPP_

#include "drivers/legacy/InputMode.hpp"

class PassthroughMode : public LegacyInputMode {
public:
    void UpdateOutputs(
        const InputState &inputs,
        OutputState &outputs,
        CommunicationBackendId backend_id
    ) override;
};

#endif
