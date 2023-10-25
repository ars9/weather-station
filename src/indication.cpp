#include "indication.hpp"

void Indication::begin()
{
    Serial.print("Initializing indication... ");
    dac_output_enable(dac_led_channel);
    Wire.begin();
    pinMode(status_led_pin, OUTPUT);
    Serial.println("Done");
}

void Indication::set_status_led_pin(uint8_t pin)
{
    status_led_pin = pin;
}

void Indication::set_dac_led_channel(dac_channel_t channel)
{
    dac_led_channel = channel;
}

void Indication::status_on()
{
    digitalWrite(status_led_pin, HIGH);
}

void Indication::status_off()
{
    digitalWrite(status_led_pin, LOW);
}

/// LED starts glowing at 128, and is fully lit at 255
void Indication::analog_led(uint8_t value)
{
    dac_output_voltage(DAC_CHANNEL_1, value);
}
