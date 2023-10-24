#include "Sensors.hpp"

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
    data.pressure = bmp.readPressure() / 100.0F;
    data.altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    return data;
}

bool Sensors::init_ds18b20()
{
    Serial.print("[Sensors] Initializing DS18B20...");
    oneWire.begin(ONE_WIRE_BUS);
    ds18b20.setOneWire(&oneWire);
    ds18b20.begin();

    if (ds18b20.getDeviceCount() > 0)
    {
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
    return lightMeter.readLightLevel();
}