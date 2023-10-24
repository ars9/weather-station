#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <WiFi.h>

#define WIFI_STATUS_CHECK_INTERVAL 1000
#define WIFI_RECONNECT_INTERVAL 5000
#define WIFI_RECONNECT_BACKOFF 500

class Network
{
public:
    Network(const char *ssid, const char *password) : ssid(ssid), password(password) {
        onConnectStart = []() {};
        onConnectFinish = []() {};
    }

    void connect();
    void checkConnection();
    void setOnConnectStart(void (*onConnectStart)());
    void setOnConnectFinish(void (*onConnectFinish)());

private:
    const char *ssid;
    const char *password;
    void (*onConnectStart)();
    void (*onConnectFinish)();
};

#endif
