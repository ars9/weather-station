#include <Indication.hpp>
#include <Network.hpp>
#include <Scheduler.hpp>
#include <Sensors.hpp>

#include "./credentials.hpp"

#define TICK_INTERVAL 100
#define SERIAL_BAUD_RATE 115200

const int LED_BUILTIN = 2;

Network network = Network(WIFI_SSID, WIFI_PASSWORD);
Indication indication = Indication();
Scheduler scheduler = Scheduler();
Sensors sensors = Sensors();

void task_bmp_read()
{
  BMP280Data bmp_data = sensors.read_bmp();

  Serial.printf("[BMP280] Temperature: %f℃\n", bmp_data.temperature);
  Serial.printf("[BMP280] Pressure: %fhPa\n", bmp_data.pressure);
  Serial.printf("[BMP280] Altitude: %fm\n", bmp_data.altitude);
}

void task_ds18b20_read()
{

  for (uint8_t i = 0; i < sensors.get_ds18b20_count(); i++)
  {
    DS18B20Data ds18b20_data = sensors.read_ds18b20(i);

    Serial.printf("[DS18B20:%d] Temperature: %f℃\n", i, ds18b20_data.temperature);
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);

  // Set up indication
  indication.set_status_led_pin(LED_BUILTIN);
  indication.set_dac_led_channel(DAC_CHANNEL_1);
  indication.begin();

  // Turn on indicator when starting to connect to WiFi
  network.setOnConnectStart([]()
                            { indication.status_on(); });

  // Turn off indicator when WiFi connection is established
  network.setOnConnectFinish([]()
                             { indication.status_off(); });

  scheduler.add_task(
      "Check WiFi connection", []()
      { network.checkConnection(); },
      30000);

  if (sensors.init_ds18b20())
  {
    scheduler.add_task("DS18B20 measurements", task_ds18b20_read, 10000);
  }

  if (sensors.init_bmp())
  {
    scheduler.add_task("BMP280 measurements", task_bmp_read, 10000);
  }

  if (sensors.init_lightmeter())
  {
    scheduler.add_task(
        "BH1750 measurements", []()
        { Serial.printf("[BH1750] Light intensity: %flx\n", sensors.read_lightmeter()); },
        10000);

    scheduler.add_task(
        "BH1750 indication", []()
        {
          float lux = sensors.read_lightmeter();

          if (lux < 100)
          {
            indication.analog_led(128);
          }
          else if (lux < 1000)
          {
            indication.analog_led(192);
          }
          else if (lux < 10000)
          {
            indication.analog_led(255);
          }
          else
          {
            indication.analog_led(0);
          } },
        100);
  }

  network.connect();
}

void loop()
{
  scheduler.tick();

  delay(TICK_INTERVAL);
}
