#include "./credentials.hpp"

#include "indication.hpp"
#include "network.hpp"
#include "sensors.hpp"

#define SERIAL_BAUD_RATE 115200

const int LED_BUILTIN = 2;

Network network = Network();
Indication indication = Indication();
Sensors sensors = Sensors();

void task_bmp280_read(void *pvParameters)
{
  for (;;)
  {
    BMP280Data bmp_data = sensors.read_bmp280();

    Serial.printf("[BMP280] Temperature: %f℃\n", bmp_data.temperature);
    Serial.printf("[BMP280] Pressure: %fPa\n", bmp_data.pressure);
    Serial.printf("[BMP280] Altitude: %fm\n", bmp_data.altitude);

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void task_bme680_read(void *pvParameters)
{
  for (;;)
  {
    BME680Data bme_data = sensors.read_bme680();

    Serial.printf("[BME680] Temperature: %f℃\n", bme_data.temperature);
    Serial.printf("[BME680] Pressure: %fPa\n", bme_data.pressure);
    Serial.printf("[BME680] Humidity: %f%%\n", bme_data.humidity);
    Serial.printf("[BME680] Gas resistance: %fΩ\n", bme_data.gas_resistance);
    Serial.printf("[BME680] Altitude: %fm\n", bme_data.altitude);

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void task_ds18b20_read(void *pvParameters)
{
  for (;;)
  {
    for (uint8_t i = 0; i < sensors.get_ds18b20_count(); i++)
    {
      DS18B20Data ds18b20_data = sensors.read_ds18b20(i);

      Serial.printf("[DS18B20:%d] Temperature: %f℃\n", i, ds18b20_data.temperature);
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void task_bh1750_read(void *pvParameters)
{
  for (;;)
  {
    float lux = sensors.read_bh1750();

    Serial.printf("[BH1750] Light intensity: %flx\n", lux);

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void task_bh1750_indication(void *pvParameters)
{
  for (;;)
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
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void task_post_sensor_data(void *pvParameters)
{
  for (;;)
  {
    Serial.println("[HTTP] Posting sensor data");

    network.post(POST_URL, sensors.get_json());

    vTaskDelay(pdMS_TO_TICKS(60000));
  }
}

void task_display_data(void *pvParameters)
{
  for (;;)
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
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void task_check_connection(void *pvParameters)
{
  for (;;)
  {
    network.checkConnection();
    vTaskDelay(pdMS_TO_TICKS(30000));
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

  indication.set_line2("DS18B20...");
  if (sensors.init_ds18b20())
  {
    xTaskCreate(task_ds18b20_read, "DS18B20 measurements", 2048, NULL, 1, NULL);
    indication.set_line2("DS18B20... Ok");
  }
  else
  {
    indication.set_line2("DS18B20... Err");
  }

  indication.set_line2("BMP280...");
  if (sensors.init_bmp280())
  {
    xTaskCreate(task_bmp280_read, "BMP280 measurements", 2048, NULL, 1, NULL);
    indication.set_line2("BMP280... Ok");
  }
  else
  {
    indication.set_line2("BMP280... Err");
  }

  indication.set_line2("BME680...");
  if (sensors.init_bme680())
  {
    xTaskCreate(task_bme680_read, "BME680 measurements", 2048, NULL, 1, NULL);
    indication.set_line2("BME680... Ok");
  }
  else
  {
    indication.set_line2("BME680... Err");
  }

  indication.set_line2("BH1750...");
  if (sensors.init_bh1750())
  {
    xTaskCreate(task_bh1750_read, "BH1750 measurements", 2048, NULL, 1, NULL);
    xTaskCreate(task_bh1750_indication, "BH1750 indication", 2048, NULL, 1, NULL);
    indication.set_line2("BH1750... Ok");
  }
  else
  {
    indication.set_line2("BH1750... Err");
  }

  xTaskCreate(task_display_data, "Display data", 20480, NULL, 1, NULL);

  indication.set_line2("Post data... Ok");
  xTaskCreate(task_post_sensor_data, "Post sensor data", 20480, NULL, 1, NULL);

  indication.set_line2("Setup WiFi connection");
  xTaskCreate(task_check_connection, "WiFi connect", 20480, NULL, 1, NULL);

  indication.set_line1("Network setup...");
  indication.set_line2("WiFi connect...");
  indication.set_line3(WIFI_SSID);
  indication.set_line1("");
  indication.set_line2("");
  indication.set_line3("");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
