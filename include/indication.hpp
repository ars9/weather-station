#ifndef INDICATION_HPP
#define INDICATION_HPP

#include <Arduino.h>
#include <Wire.h>
#include <driver/dac.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include <SPI.h>

#define OLED_RESET 4

class Indication
{
public:
    void begin();
    void status_on();
    void status_off();
    void analog_led(uint8_t value);
    void set_status_led_pin(uint8_t pin);
    void set_dac_led_channel(dac_channel_t channel);

    void refresh_display();
    void set_line1(const char* text);
    void set_line2(const char* text);
    void set_line3(const char* text);
    void render_page();

    uint8_t display_page = 0;
    std::string display_content = "";
private:
    uint8_t status_led_pin;
    dac_channel_t dac_led_channel;

    bool is_display_on = false;
    Adafruit_SSD1306 display;

    const char* line1 = "Init...";
    const char* line2 = "";
    const char* line3 = "";
};

#endif
