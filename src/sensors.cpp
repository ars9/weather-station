#include "sensors.hpp"

bool Sensors::i2c_scanner() {
    Serial.println("[Sensors] Scanning I2C bus...");
    byte error, address;
    int nDevices;

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.printf("[Sensors] I2C device found at address 0x%x\n", address);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.printf("[Sensors] Unknown error at address 0x%x\n", address);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("[Sensors] No I2C devices found\n");
        return false;
    }
    else
    {
        Serial.println("[Sensors] Done");
        return true;
    }
}

bool Sensors::init_bmp280()
{
    Serial.print("[Sensors] Initializing BMP280...");
    if (bmp280.begin(0x76, 0x60))
    {

        Serial.println(" Done");
        is_bmp280_on = true;
        return true;
    }
    else
    {
        Serial.println(" Failed");
        Serial.printf("Status: %d\n", bmp280.sensorID());
        return false;
    }
}

BMP280Data Sensors::read_bmp280()
{
    BMP280Data data;
    data.temperature = bmp280.readTemperature();
    data.pressure = bmp280.readPressure();
    data.altitude = bmp280.readAltitude(SEALEVELPRESSURE_HPA);

    this->data.bmp280 = data;

    return data;
}

bool Sensors::init_bme680()
{
    Serial.print("[Sensors] Initializing BME680...");
    if (bme680.begin())
    {
        bme680.setTemperatureOversampling(BME680_OS_8X);
        bme680.setHumidityOversampling(BME680_OS_2X);
        bme680.setPressureOversampling(BME680_OS_4X);
        bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme680.setGasHeater(320, 150); // 320*C for 150 ms
        Serial.println(" Done");
        is_bme680_on = true;
        return true;
    }
    else
    {
        Serial.println(" Failed");
        return false;
    }
}

BME680Data Sensors::read_bme680()
{
    BME680Data data;
    data.temperature = bme680.temperature;
    data.pressure = bme680.pressure;
    data.humidity = bme680.humidity;
    data.gas_resistance = bme680.gas_resistance;
    data.altitude = bme680.readAltitude(SEALEVELPRESSURE_HPA);

    this->data.bme680 = data;

    return data;
}

bool Sensors::init_ds18b20()
{
    Serial.print("[Sensors] Initializing DS18B20...");
    oneWire.begin(ONE_WIRE_BUS);
    ds18b20.setOneWire(&oneWire);
    ds18b20.begin();

    uint8_t device_count = ds18b20.getDeviceCount();
    if (device_count > 0)
    {
        this->data.ds18b20.resize(device_count);
        ds18b20.requestTemperatures();
        Serial.printf(" Done (%d devices found)\n", ds18b20.getDeviceCount());
        is_ds18b20_on = true;
        return true;
    }
    else
    {
        Serial.println(" Failed");
        return false;
    }
}

DS18B20Data Sensors::read_ds18b20(uint8_t index)
{
    DS18B20Data data;
    data.temperature = ds18b20.getTempCByIndex(index);

    this->data.ds18b20[index] = data;

    return data;
}

uint8_t Sensors::get_ds18b20_count()
{
    return ds18b20.getDeviceCount();
}

bool Sensors::init_bh1750()
{
    Serial.print("[Sensors] Initializing BH1750...");
    if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
    {
        Serial.println(" Done");
        is_bh1750_on = true;
        return true;
    }
    else
    {
        Serial.println(" Failed");
        return false;
    }
}

float Sensors::read_bh1750()
{
    this->data.bh1750 = bh1750.readLightLevel();
    return bh1750.readLightLevel();
}

JsonDocument Sensors::get_json()
{
    float totalTemperature = this->data.bmp280.temperature;
    for (const auto &sensor : this->data.ds18b20)
    {
        totalTemperature += sensor.temperature;
    }

    float temperature = totalTemperature / (this->data.ds18b20.size() + 1);
    float pressure = this->data.bmp280.pressure;
    float altitude = this->data.bmp280.altitude;
    float bh1750 = this->data.bh1750;

    JsonDocument doc;

    doc["temperature"] = temperature;
    doc["pressure"] = pressure;
    doc["altitude"] = altitude;
    doc["lightmeter"] = bh1750;

    return doc;
}

BMP280Data Sensors::get_bmp280()
{
    return this->data.bmp280;
}

BME680Data Sensors::get_bme680()
{
    return this->data.bme680;
}

std::vector<DS18B20Data> Sensors::get_ds18b20()
{
    return this->data.ds18b20;
}

float Sensors::get_bh1750()
{
    return this->data.bh1750;
}