#ifndef _GLYPH_MATRIX_INPUT_H_
#define _GLYPH_MATRIX_INPUT_H_

#include "gpaddon.h"
#include "gamepad/GamepadState.h"

#include <stdint.h>
#include <string>

#ifndef GLYPH_MATRIX_INPUT_ENABLED
#define GLYPH_MATRIX_INPUT_ENABLED 0
#endif

#ifndef GLYPH_MATRIX_DEBOUNCE_MS
#define GLYPH_MATRIX_DEBOUNCE_MS 5
#endif

class GlyphMatrixInput : public GPAddon
{
public:
    struct DebounceState {
        bool stable;
        bool observed;
        uint32_t changedAt;
    };

    virtual bool available();
    virtual void setup();
    virtual void process();
    virtual void preprocess();
    virtual void postprocess(bool sent);
    virtual void reinit();
    virtual std::string name();

private:
    void ensureProfiles();
    void applyProfileOptions();
    void handleMenuControls();
    void scan();
    void apply(GamepadState& state);
    void setCell(uint8_t row, uint8_t col, bool pressed, uint32_t now);
    void clearPressed();
};

#endif
