#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <Adafruit_BMP280.h>
#include <Adafruit_BME680.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <vector>
#include <sstream>

#define BMP_SDA 21
#define BMP_SCL 22
#define BMP_ADDRESS 0x76

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// HW239A

#define ONE_WIRE_BUS 15
#define SEALEVELPRESSURE_HPA (1013.25)

struct BMP280Data
{
    float temperature;
    float pressure;
    float altitude;
};

struct BME680Data
{
    float temperature;
    float pressure;
    float humidity;
    float gas_resistance;
    float altitude;
};

struct DS18B20Data
{
    float temperature;
};

struct SensorData
{
    BMP280Data bmp280;
    BME680Data bme680;
    std::vector<DS18B20Data> ds18b20;
    float bh1750;
};

class Sensors
{
public:
    bool i2c_scanner();
    bool init_bmp280();
    bool init_bme680();
    bool init_ds18b20();
    bool init_bh1750();

    BMP280Data read_bmp280();
    BME680Data read_bme680();
    DS18B20Data read_ds18b20(uint8_t index = 0);
    uint8_t get_ds18b20_count();
    float read_bh1750();

    const char* get_json();
    BMP280Data get_bmp280();
    BME680Data get_bme680();
    std::vector<DS18B20Data> get_ds18b20();
    float get_bh1750();

    bool is_bmp280_on = false;
    bool is_bme680_on = false;
    bool is_ds18b20_on = false;
    bool is_bh1750_on = false;

private:
    Adafruit_BMP280 bmp280;
    Adafruit_BME680 bme680;
    OneWire oneWire;
    DallasTemperature ds18b20;
    BH1750 bh1750;

    // Accumulate latest sensor data here
    std::string jsonOutput;
    SensorData data;
};

#endif
