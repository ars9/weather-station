#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#define WIFI_STATUS_CHECK_INTERVAL 1000
#define WIFI_RECONNECT_INTERVAL 5000
#define WIFI_RECONNECT_BACKOFF 500

extern const char *rootCACertificate;

class Network
{
public:
    Network();

    void setWiFiCredentials(const char *ssid, const char *password);
    void setBearerToken(const char *bearerToken);

    void connect();
    void checkConnection();
    void setOnConnectStart(void (*onConnectStart)());
    void setOnConnectFinish(void (*onConnectFinish)());

    bool post(const char *url, const char *payload);
    void setOnPostStart(void (*onPostStart)());
    void setOnPostFinish(void (*onPostFinish)());

private:
    const char *ssid;
    const char *password;
    const char *bearerToken;

    WiFiClientSecure secureClient;

    void (*onConnectStart)();
    void (*onConnectFinish)();
    void (*onPostStart)();
    void (*onPostFinish)();
};

#endif
