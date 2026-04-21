#ifndef _GLYPH_INPUT_SCREEN_H_
#define _GLYPH_INPUT_SCREEN_H_

#include <stdint.h>
#include <string>

#include "display/ui/elements/GPScreen.h"
#include "gamepad/GamepadState.h"
#include "enums.pb.h"

class GlyphInputScreen : public GPScreen {
public:
    struct ButtonDot {
        const char* label;
        uint8_t x;
        uint8_t y;
        uint8_t radius;
        uint32_t buttonMask;
        uint8_t dpadMask;
        uint16_t auxMask;
    };

    GlyphInputScreen() {}
    GlyphInputScreen(GPGFX* renderer) { setRenderer(renderer); }
    virtual ~GlyphInputScreen() {}

    virtual void init();
    virtual void shutdown();
    virtual int8_t update();

protected:
    virtual void drawScreen();

private:
    std::string inputModeName(InputMode mode) const;
    std::string socdName(SOCDMode mode) const;
    const ButtonDot* activeDots(size_t& count) const;
    void drawDot(const ButtonDot& dot, const GamepadState& state);
    bool dotPressed(const ButtonDot& dot, const GamepadState& state) const;

    Gamepad* gamepad = nullptr;
};

#endif
