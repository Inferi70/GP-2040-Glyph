#ifndef _LEGACY_GAMECUBEBACKEND_HPP_
#define _LEGACY_GAMECUBEBACKEND_HPP_

#include "drivers/console/GamecubeConsole.hpp"
#include "drivers/legacy/CommunicationBackend.hpp"

class GamecubeBackend : public CommunicationBackend {
public:
    static constexpr uint32_t kNoPollYieldTimeoutUs = 5000;

    GamecubeBackend(
        InputState &inputs,
        InputSource **input_sources,
        size_t input_source_count,
        uint data_pin,
        PIO pio = pio0,
        int sm = -1,
        int offset = -1
    );
    CommunicationBackendId BackendId() override;
    bool Detect() override;
    void SendReport() override;
    bool sawPollLastCycle() const { return _sawPollLastCycle; }

private:
    GamecubeConsole _gamecube;
    gc_report_t _report;
    bool _sawPollLastCycle = false;
};

#endif
