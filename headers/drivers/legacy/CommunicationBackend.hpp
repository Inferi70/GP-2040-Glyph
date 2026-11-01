#ifndef _LEGACY_COMMUNICATIONBACKEND_HPP_
#define _LEGACY_COMMUNICATIONBACKEND_HPP_

#include "drivers/legacy/InputMode.hpp"
#include "drivers/legacy/InputSourceManager.hpp"
#include <cstddef>

class CommunicationBackend {
public:
    CommunicationBackend(
        InputState &inputs,
        InputSource **input_sources,
        size_t input_source_count
    );
    virtual ~CommunicationBackend() {}

    void ScanInputs();
    void ScanInputs(InputScanSpeed input_source_filter);
    virtual void UpdateOutputs();
    virtual CommunicationBackendId BackendId();
    virtual void SetGameMode(LegacyInputMode *gamemode);
    virtual LegacyInputMode *CurrentGameMode();
    virtual bool Detect() { return true; }
    virtual void SendReport() = 0;

protected:
    InputState &_inputs;
    InputSourceManager _input_source_manager;
    OutputState _outputs;
    LegacyInputMode *_gamemode = nullptr;
};

#endif
