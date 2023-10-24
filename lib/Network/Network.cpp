#include "Network.hpp"

void Network::connect()
{
    onConnectStart();
    Serial.printf("[WiFi] Connecting to \"%s\"...", ssid);

    unsigned long startTime = millis();
    unsigned long reconnectInterval = WIFI_RECONNECT_INTERVAL;

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > reconnectInterval)
        {
            Serial.printf("\n[WiFi] WiFi connection failed. Attempting to reconnect in %dms\n", reconnectInterval);
            WiFi.disconnect();
            onConnectFinish();
            delay(WIFI_STATUS_CHECK_INTERVAL);
            onConnectStart();
            WiFi.begin(ssid, password);
            startTime = millis();
            reconnectInterval += WIFI_RECONNECT_BACKOFF;
        }
        delay(WIFI_STATUS_CHECK_INTERVAL);
        Serial.print(".");
    }
    Serial.println(" Done");

    Serial.printf("[WiFi] Local IP obtained: %s\n", WiFi.localIP().toString().c_str());
    onConnectFinish();
}

void Network::checkConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFi] Connection lost. Attempting to reconnect...");
    this->connect();
  }
  else
  {
    Serial.println("[WiFi] Connection OK");
  }
}

void Network::setOnConnectStart(void (*onConnectStart)())
{
    this->onConnectStart = onConnectStart;
}

void Network::setOnConnectFinish(void (*onConnectFinish)())
{
    this->onConnectFinish = onConnectFinish;
}
