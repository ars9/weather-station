#include "sensors.hpp"

bool Sensors::init_bmp()
{

    Serial.print("[Sensors] Initializing BMP280...");
    if (bmp.begin(0x76, 0x60))
    {

        Serial.println(" Done");
        return true;
    }
    else
    {
        Serial.println(" Failed");
        Serial.printf("Status: %d\n", bmp.sensorID());
        return false;
    }
}

BMP280Data Sensors::read_bmp()
{
    BMP280Data data;
    data.temperature = bmp.readTemperature();
    data.pressure = bmp.readPressure();
    data.altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);

    this->data.bmp = data;

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

bool Sensors::init_lightmeter()
{
    Serial.print("[Sensors] Initializing BH1750...");
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
    {
        Serial.println(" Done");
        return true;
    }
    else
    {
        Serial.println(" Failed");
        return false;
    }
}

float Sensors::read_lightmeter()
{
    this->data.lightmeter = lightMeter.readLightLevel();
    return lightMeter.readLightLevel();
}

const char *Sensors::get_json()
{
    std::ostringstream stream;
    float totalTemperature = this->data.bmp.temperature;
    for (const auto &sensor : this->data.ds18b20)
    {
        totalTemperature += sensor.temperature;
    }

    float temperature = totalTemperature / (this->data.ds18b20.size() + 1);
    float pressure = this->data.bmp.pressure;
    float altitude = this->data.bmp.altitude;
    float lightmeter = this->data.lightmeter;

    stream << "{";
    stream << "\"temperature\": " << temperature << ",";
    stream << "\"pressure\": " << pressure << ",";
    stream << "\"altitude\": " << altitude << ",";
    stream << "\"lightmeter\": " << lightmeter;
    stream << "}";

    jsonOutput = stream.str();
    return jsonOutput.c_str();
}
