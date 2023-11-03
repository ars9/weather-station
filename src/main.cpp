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

void task_bme680_read()
{
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

void task_display_data()
{
  std::ostringstream stream;
  stream.precision(3);
  if (indication.display_page == 0)
  {
    if (sensors.is_bmp280_on)
    {
      stream << "BMP280" << std::endl;
      stream << "T:" << sensors.get_bmp280().temperature << " C" << std::endl;
      stream << "P:" << sensors.get_bmp280().pressure / 1000 << " kPa" << std::endl;
    }
    else
    {
      indication.display_page++;
    }
  }

  if (indication.display_page == 1)
  {
    if (sensors.is_bme680_on)
    {
      stream << "BME680" << std::endl;
      stream << "T:" << sensors.get_bme680().temperature << " C ";
      stream << "H:" << sensors.get_bme680().humidity << "%" << std::endl;
      stream << "P:" << sensors.get_bme680().pressure / 1000 << " kPa" << std::endl;
    }
    else
    {
      indication.display_page++;
    }
  }

  if (indication.display_page == 2)
  {
    if (sensors.is_bh1750_on)
    {
      stream << "BH1750" << std::endl;
      stream << "L:" << sensors.get_bh1750() << " lx" << std::endl;
    }
    else
    {
      indication.display_page++;
    }
  }

  if (indication.display_page == 3)
  {
    if (sensors.is_ds18b20_on)
    {
      stream << "DS18B20" << std::endl;
      for (uint8_t i = 0; i < sensors.get_ds18b20().size(); i++)
      {
        stream << "T:" << sensors.get_ds18b20()[i].temperature << " C" << std::endl;
      }
    }
    else
    {
      indication.display_page++;
    }
  }

  indication.display_content = stream.str();
  indication.render_page();
  indication.display_page++;

  indication.display_page = indication.display_page % 4;
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);

  // Set up indication
  indication.set_status_led_pin(LED_BUILTIN);
  indication.set_dac_led_channel(DAC_CHANNEL_1);
  indication.begin();

  indication.set_line1("Scan I2C bus...");
  sensors.i2c_scanner();

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

  indication.set_line1("Task setup...");

  indication.set_line2("WiFi connection");
  scheduler.add_task(
      "Check WiFi connection", []()
      { network.checkConnection(); },
      30000);

  indication.set_line2("DS18B20...");
  if (sensors.init_ds18b20())
  {
    scheduler.add_task("DS18B20 measurements", task_ds18b20_read, 10000);
    indication.set_line2("DS18B20... Ok");
  }
  else
  {
    indication.set_line2("DS18B20... Err");
  }

  indication.set_line2("BMP280...");
  if (sensors.init_bmp280())
  {
    scheduler.add_task("BMP280 measurements", task_bmp280_read, 10000);
    indication.set_line2("BMP280... Ok");
  }
  else
  {
    indication.set_line2("BMP280... Err");
  }

  indication.set_line2("BME680...");
  if (sensors.init_bme680())
  {
    scheduler.add_task("BME680 measurements", task_bme680_read, 10000);
    indication.set_line2("BME680... Ok");
  }
  else
  {
    indication.set_line2("BME680... Err");
  }

  indication.set_line2("BH1750...");
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

    indication.set_line2("BH1750... Ok");
  }
  else
  {
    indication.set_line2("BH1750... Err");
  }

  scheduler.add_task("Display data", task_display_data, 3000);

  indication.set_line2("Post data... Ok");
  scheduler.add_task("Post sensor data", task_post_sensor_data, 60000);

  indication.set_line1("Network setup...");
  indication.set_line2("WiFi connect...");
  indication.set_line3(WIFI_SSID);
  network.connect();
  indication.set_line1("");
  indication.set_line2("");
  indication.set_line3("");
}

void loop()
{
  scheduler.tick();

  delay(TICK_INTERVAL);
}
