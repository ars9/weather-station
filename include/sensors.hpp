#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <vector>
#include <sstream>

#define BMP_SDA 21
#define BMP_SCL 22
#define BMP_ADDRESS 0x76

#define ONE_WIRE_BUS 15
#define SEALEVELPRESSURE_HPA (1013.25)

struct BMP280Data
{
    float temperature;
    float pressure;
    float altitude;
};

struct DS18B20Data
{
    float temperature;
};

struct SensorData
{
    BMP280Data bmp;
    std::vector<DS18B20Data> ds18b20;
    float lightmeter;
};

class Sensors
{
public:
    bool init_bmp();
    bool init_ds18b20();
    bool init_lightmeter();

    BMP280Data read_bmp();
    DS18B20Data read_ds18b20(uint8_t index = 0);
    uint8_t get_ds18b20_count();
    float read_lightmeter();

    const char* get_json();

private:
    Adafruit_BMP280 bmp;
    OneWire oneWire;
    DallasTemperature ds18b20;
    BH1750 lightMeter;

    // Accumulate latest sensor data here
    std::string jsonOutput;
    SensorData data;
};

#endif
