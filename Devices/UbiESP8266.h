#ifndef __UBIESP8266_H__
#define __UBIESP8266_H__

#include <ESP8266WiFi.h>
#include "../UbiDevice.h"

class UbiESP8266 : public UbiDevice
{

public:
    static UbiDevice *getDeviceInstance();
    ~UbiESP8266();

    bool connect();
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
    char *_ssid;
    char *_password;
    uint32_t _maxConnectionRetries{10};
    WiFiClientSecure _secureClient;
    WiFiClient _nonSecureClient;
    WiFiClient* _client;
    Session _session;
    X509List _certs;
    int _timeout{5000};
    unsigned long _timerToSync{millis()};

private:
    UbiESP8266();
};

#ifndef _ONLY_DECLARATIONS_

void UbiESP8266::init()
{
    if(_protocol == UBI_HTTPS)
    {
        UbiUtils::syncronizeTime(_debug);
        _secureClient.setSession(&_session);
        UbiUtils::loadCerts(_certs, UBI_CA_CERT_1, UBI_CA_CERT_LEN_1, UBI_CA_CERT_2, UBI_CA_CERT_LEN_2, _debug);
        _secureClient.setTrustAnchors(&_certs);
        _client = &_secureClient;
    }
    else
    {
        _client = &_nonSecureClient;
    }
}

UbiESP8266::UbiESP8266(/* args */)
{
    _userAgent = const_cast<char *>(ESP8266_USER_AGENT);
}

UbiESP8266::~UbiESP8266()
{
    delete UbiDevice::_device;
}

UbiDevice *UbiESP8266::getDeviceInstance()
{
    if (UbiDevice::_device == nullptr)
    {
        UbiDevice::_device = static_cast<UbiDevice *>(new UbiESP8266);
    }
    return UbiDevice::_device;
}

bool UbiESP8266::connect()
{
    bool connected{false};
    uint32_t reconnectionAttempt{0};
    _ssid = _credentials[0];
    _password = _credentials[1];

    if (_debug)
    {
        Serial.print("\nconnecting esp32 cam to local WiFi: ");
        Serial.println(_ssid);
    }
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED && reconnectionAttempt < _maxConnectionRetries)
    {
        delay(500);
        Serial.print(".");
        reconnectionAttempt++;
    }
    if (WiFi.status() == WL_NO_SSID_AVAIL)
    {
        Serial.println("Your network SSID cannot be reached");
        return false;
    }
    if (WiFi.status() == WL_CONNECT_FAILED)
    {
        Serial.println("Network password incorrect");
        return false;
    }
    WiFi.setAutoReconnect(true);
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    return true;
}

bool UbiESP8266::serverConnected()
{
    return _client->connected();
}

bool UbiESP8266::reconnect()
{
    uint8_t attempts = 0;
    while (!_client->connected() && attempts < _maxConnectionRetries)
    {
        if (_debug)
        {
            Serial.print(F("Trying to connect to "));
            Serial.print(_host);
            Serial.print(F(" , attempt number: "));
            Serial.println(attempts);
        }
        if(_client->connect(_host, _port))
        {
            if (_debug)
                Serial.println(F("Reconnect successfull"));
            return true;
        }
        attempts += 1;
        delay(1000);
    }
    return false;
}

bool UbiESP8266::sendHttp(const char *device_label, const char *deviceName, const char *payload)
{
    if(_protocol == UBI_HTTPS)
        if (!UbiUtils::preConnectionChecks(_certs, _timerToSync, UbiUtils::syncronizeTime, _debug))
            return false;

    if (!_client->connected())
    {
        if (_debug)
        {
            Serial.println("Server not connected!!!");
            Serial.print("Trying to reconnect to host: ");
            Serial.println(_host);
            Serial.print(F("On Port: "));
            Serial.println(_port);
        }
        if (!reconnect())
        {
            if (_debug)
                Serial.println("Reconnection failed");
            return false;
        }
    }

    bool result{false};
    int contentLength{strlen(payload)};

    _client->print(F("POST /api/v1.6/devices/"));
    _client->print(device_label);
    _client->print(F(" HTTP/1.1\r\n"));
    _client->print(F("Host: "));
    _client->print(_host);
    _client->print(F("\r\n"));
    _client->print(F("User-Agent: "));
    _client->print(_userAgent);
    _client->print(F("\r\n"));
    _client->print(F("X-Auth-Token: "));
    _client->print(_token);
    _client->print(F("\r\n"));
    _client->print(F("Connection: close\r\n"));
    _client->print(F("Content-Type: application/json\r\n"));
    _client->print(F("Content-Length: "));
    _client->print(contentLength);
    _client->print(F("\r\n\r\n"));
    _client->print(payload);
    _client->print(F("\r\n"));
    _client->flush();

    if (UbiUtils::waitServerAnswer(*_client, _debug))
    {
        if (_debug)
        {
            Serial.println(F("\nUbidots' Server response:\n"));
            while (_client->available())
            {
                char c = _client->read();
                Serial.print(c);
            }
        }
        result = true;
    }
    else
    {
        if (_debug)
        {
            Serial.println(F("Could not read server's response"));
        }
    }
    _client->stop();
    return result;
    return true;
}

bool UbiESP8266::sendTcp(const char *device_label, const char *deviceName, const char *payload)
{
    return true;
}
bool UbiESP8266::sendUdp(const char *device_label, const char *deviceName, const char *payload)
{
    return true;
}
double UbiESP8266::getHttp(const char *device_label, const char *variable_label)
{

    return 99;
}
double UbiESP8266::getTcp(const char *device_label, const char *variable_label)
{
    return 99;
}
void UbiESP8266::getUniqueID(char *ID)
{
    if (_debug)
        Serial.println("getting MAC");
    if (strcmp(_uniqueID, "undefined") == 0)
    {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        sprintf(_uniqueID, "%.2X%.2X%.2X%.2X%.2X%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    ID = _uniqueID;
    if (_debug)
    {
        Serial.println("Done! MAC is: ");
        Serial.println(ID);
    }
}

#endif

#endif // __UBIESP8266_H__