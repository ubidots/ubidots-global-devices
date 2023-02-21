#ifndef _UBI_ESP32_H_
#define _UBI_ESP32_H_

#include "../UbiDevice.h"
#include "WiFi.h"
//#include <WiFiClientSecure.h>

class UbiESP32 : public UbiDevice
{


public:

    static UbiDevice *getDeviceInstance();
    ~UbiESP32();


    bool connect();
    bool connected();
    bool reconnect();
    bool sendHttp(const char *device_label, const char *deviceName, const char *payload);
    bool sendTcp(const char *device_label, const char *deviceName, const char *payload);
    bool sendUdp(const char *device_label, const char *deviceName, const char *payload);
    bool serverConnected();
    double getHttp(const char *device_label, const char *variable_label);
    double getTcp(const char *device_label, const char *variable_label);
    void getUniqueID(char *ID);
    void init();

    friend class UbiUtils;

protected:

    char* _ssid;
    char* _password;
    uint32_t _maxConnectionRetries{10};
    WiFiClient _client;

private:
    UbiESP32();

};

#ifndef _ONLY_DECLARATIONS_

void UbiESP32::init()
{

}

UbiESP32::UbiESP32(/* args */)
{
    _userAgent = const_cast<char*>(ESP32_USER_AGENT);
}

UbiESP32::~UbiESP32()
{
    delete UbiDevice::_device;
}


UbiDevice *UbiESP32::getDeviceInstance()
{
    if(UbiDevice::_device == nullptr)
    {
        UbiDevice::_device = static_cast<UbiDevice*>(new UbiESP32);
    }
    return UbiDevice::_device;
}

bool UbiESP32::connect()
{
    bool connected{false};
    uint32_t reconnectionAttempt{0};
    _ssid = _credentials[0];
    _password = _credentials[1];

    if(_debug)
    {
        Serial.print("\nconnecting esp32 cam to local WiFi: ");
		Serial.println(_ssid);
    }
    WiFi.begin(_ssid, _password);

    return connected;
}

bool UbiESP32::serverConnected()
{
    return false;
}

bool UbiESP32::connected()
{
    return _client->connected();
}

bool UbiESP32::reconnect()
{
    return connect();
}


bool UbiESP32::sendHttp(const char *device_label, const char *deviceName, const char *payload)
{

    return 0;
}
bool UbiESP32::sendTcp(const char *device_label, const char *deviceName, const char *payload)
{

    return 0;
}
bool UbiESP32::sendUdp(const char *device_label, const char *deviceName, const char *payload)
{

    return 0;
}
double UbiESP32::getHttp(const char *device_label, const char *variable_label)
{

    return 0;
}
double UbiESP32::getTcp(const char *device_label, const char *variable_label)
{
    return 0;
}
void UbiESP32::getUniqueID(char *ID)
{

}

#endif

#endif
