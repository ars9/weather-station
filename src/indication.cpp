#include "indication.hpp"

void Indication::begin()
{
    Serial.println("Initializing indication...");

    if (dac_output_enable(dac_led_channel) == ESP_OK)
    {
        Serial.println("[OK] DAC");
    }
    else
    {
        Serial.println("[ER] DAC");
    }

    if (Wire.begin())
    {
        Serial.println("[OK] I2C");
        pinMode(status_led_pin, OUTPUT);
    }
    else
    {
        Serial.println("[ER] I2C");
    }

    display = Adafruit_SSD1306(OLED_RESET);
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.printf("[OK] OLED %d x %d\n", display.width(), display.height());
        is_display_on = true;
        refresh_display();
    }
    else
    {
        Serial.println("[ER] OLED");
        is_display_on = false;
    }
}

void Indication::refresh_display()
{
    if (is_display_on)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(line1);
        display.setCursor(0, 10);
        display.println(line2);
        display.setCursor(0, 20);
        display.println(line3);
        display.display();
    }
}

void Indication::set_line1(const char* text)
{
    if (is_display_on)
    {
        line1 = text;
        refresh_display();
    }
}

void Indication::set_line2(const char* text)
{
    if (is_display_on)
    {
        line2 = text;
        refresh_display();
    }
}

void Indication::set_line3(const char* text)
{
    if (is_display_on)
    {
        line3 = text;
        refresh_display();
    }
}

void Indication::render_page() {
    if (is_display_on)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(display_content.c_str());
        display.display();
    }
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
    if (is_display_on) {
        display.drawPixel(0, 0, WHITE);
        display.display();
    }
    digitalWrite(status_led_pin, HIGH);
}

void Indication::status_off()
{
    if (is_display_on) {
        display.drawPixel(display.width() - 1, 0, BLACK);
        display.display();
    }
    digitalWrite(status_led_pin, LOW);
}

/// LED starts glowing at 128, and is fully lit at 255
void Indication::analog_led(uint8_t value)
{
    dac_output_voltage(DAC_CHANNEL_1, value);
}
