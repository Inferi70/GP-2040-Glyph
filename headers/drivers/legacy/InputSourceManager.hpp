#ifndef _LEGACY_INPUTSOURCEMANAGER_HPP_
#define _LEGACY_INPUTSOURCEMANAGER_HPP_

#include "drivers/legacy/InputSource.hpp"
#include <cstddef>

class InputSourceManager {
public:
    InputSourceManager(InputSource **input_sources, size_t input_source_count);
    void ScanInputs(InputState &inputs);
    void ScanInputs(InputState &inputs, InputScanSpeed input_source_filter);

private:
    InputSource **_input_sources;
    size_t _input_source_count;
};

#endif
