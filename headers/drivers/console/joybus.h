#ifndef _JOYBUS_JOYBUS_H
#define _JOYBUS_JOYBUS_H

#include <hardware/pio.h>
#include <pico/stdlib.h>

typedef struct {
    uint pin;
    PIO pio;
    uint sm;
    uint offset;
    pio_sm_config config;
} joybus_port_t;

#ifdef __cplusplus
extern "C" {
#endif

uint joybus_port_init(joybus_port_t *port, uint pin, PIO pio, int sm, int offset);
void joybus_port_terminate(joybus_port_t *port);
void joybus_port_reset(joybus_port_t *port);
uint joybus_send_receive(
    joybus_port_t *port,
    uint8_t *message,
    uint message_len,
    uint8_t *response_buf,
    uint response_len,
    uint read_timeout_us
);
void joybus_send_bytes(joybus_port_t *port, uint8_t *bytes, uint len);
void joybus_send_byte(joybus_port_t *port, uint8_t byte, bool stop);
uint joybus_receive_bytes(
    joybus_port_t *port,
    uint8_t *buf,
    uint len,
    uint64_t timeout_us,
    bool first_byte_can_timeout
);
uint8_t joybus_receive_byte(joybus_port_t *port);

#ifdef __cplusplus
}
#endif

#endif
