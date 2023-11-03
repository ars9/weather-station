#include "./credentials.hpp"

#include "indication.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "sensors.hpp"

#define TICK_INTERVAL 100
#define SERIAL_BAUD_RATE 115200

const int LED_BUILTIN = 2;

Network network = Network();
Indication indication = Indication();
Scheduler scheduler = Scheduler();
Sensors sensors = Sensors();

void task_bmp280_read()
{
  BMP280Data bmp_data = sensors.read_bmp280();

  Serial.printf("[BMP280] Temperature: %f℃\n", bmp_data.temperature);
  Serial.printf("[BMP280] Pressure: %fPa\n", bmp_data.pressure);
  Serial.printf("[BMP280] Altitude: %fm\n", bmp_data.altitude);
}

void task_bme680_read() {
  BME680Data bme_data = sensors.read_bme680();

  Serial.printf("[BME680] Temperature: %f℃\n", bme_data.temperature);
  Serial.printf("[BME680] Pressure: %fPa\n", bme_data.pressure);
  Serial.printf("[BME680] Humidity: %f%%\n", bme_data.humidity);
  Serial.printf("[BME680] Gas resistance: %fΩ\n", bme_data.gas_resistance);
  Serial.printf("[BME680] Altitude: %fm\n", bme_data.altitude);
}

void task_ds18b20_read()
{
  for (uint8_t i = 0; i < sensors.get_ds18b20_count(); i++)
  {
    DS18B20Data ds18b20_data = sensors.read_ds18b20(i);

    Serial.printf("[DS18B20:%d] Temperature: %f℃\n", i, ds18b20_data.temperature);
  }
}

void task_post_sensor_data()
{
  Serial.println("[HTTP] Posting sensor data");

  network.post(POST_URL, sensors.get_json());
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);

  // Set up indication
  indication.set_status_led_pin(LED_BUILTIN);
  indication.set_dac_led_channel(DAC_CHANNEL_1);
  indication.begin();

  network.setWiFiCredentials(WIFI_SSID, WIFI_PASSWORD);
  network.setBearerToken(BEARER_TOKEN);

  network.setOnConnectStart([]()
                            { indication.status_on(); });

  network.setOnConnectFinish([]()
                             { indication.status_off(); });

  network.setOnPostStart([]()
                         { indication.status_on(); });

  network.setOnPostFinish([]()
                          { indication.status_off(); });

  scheduler.add_task(
      "Check WiFi connection", []()
      { network.checkConnection(); },
      30000);

  if (sensors.init_ds18b20())
  {
    scheduler.add_task("DS18B20 measurements", task_ds18b20_read, 10000);
  }

  if (sensors.init_bmp280())
  {
    scheduler.add_task("BMP280 measurements", task_bmp280_read, 10000);
  }

  if (sensors.init_bme680())
  {
    scheduler.add_task("BME680 measurements", task_bme680_read, 10000);
  }

  if (sensors.init_bh1750())
  {
    scheduler.add_task(
        "BH1750 measurements", []()
        { Serial.printf("[BH1750] Light intensity: %flx\n", sensors.read_bh1750()); },
        10000);

    scheduler.add_task(
        "BH1750 indication", []()
        {
          float lux = sensors.read_bh1750();

          if (lux <= 0)
          {
            indication.analog_led(0);
          }
          else
          {
            float logLux = log10(lux);
            float scaledLux = map(logLux, -1, 4, 100, 255);
            indication.analog_led(scaledLux);
          } },
        100);
  }

  scheduler.add_task("Post sensor data", task_post_sensor_data, 60000);

  network.connect();
}

void loop()
{
  scheduler.tick();

  delay(TICK_INTERVAL);
}
