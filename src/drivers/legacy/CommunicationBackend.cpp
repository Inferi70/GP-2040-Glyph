#include "drivers/legacy/CommunicationBackend.hpp"

CommunicationBackend::CommunicationBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count
)
    : _inputs(inputs),
      _input_source_manager(input_sources, input_source_count) {}

void CommunicationBackend::ScanInputs() {
    _input_source_manager.ScanInputs(_inputs);
}

void CommunicationBackend::ScanInputs(InputScanSpeed input_source_filter) {
    _input_source_manager.ScanInputs(_inputs, input_source_filter);
}

void CommunicationBackend::UpdateOutputs() {
    if (_gamemode != nullptr) {
        _gamemode->UpdateOutputs(_inputs, _outputs, BackendId());
    }
}

CommunicationBackendId CommunicationBackend::BackendId() {
    return COMMS_BACKEND_UNSPECIFIED;
}

void CommunicationBackend::SetGameMode(LegacyInputMode *gamemode) {
    _gamemode = gamemode;
}

LegacyInputMode *CommunicationBackend::CurrentGameMode() {
    return _gamemode;
}
