#ifndef INDICATION_HPP
#define INDICATION_HPP

#include <Arduino.h>
#include <Wire.h>
#include <driver/dac.h>

class Indication
{
public:
    void begin();
    void status_on();
    void status_off();
    void analog_led(uint8_t value);
    void set_status_led_pin(uint8_t pin);
    void set_dac_led_channel(dac_channel_t channel);

private:
    uint8_t status_led_pin;
    dac_channel_t dac_led_channel;
};

#endif
