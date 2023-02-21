#ifndef _UBI_ARDUINO_MKR_GSM_H_
#define _UBI_ARDUINO_MKR_GSM_H_

#include "MKRGSM.h"
#include "../UbiDevice.h"

class UbiArduino_MKR_GSM : public UbiDevice
{
public:
    static UbiDevice *getDeviceInstance();
    ~UbiArduino_MKR_GSM();

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

    char *_APN;
    char *_login;
    char *_password;
    char *_pin;
    uint32_t _maxConnectionRetries{10};

    GPRS _gprs;
    GSM _gsm;
    GSMClient _client;
    GSMModem _modem;
    GSMUDP _udp;

private:
    UbiArduino_MKR_GSM();
};

#ifndef _MKR_GSM_ONLY_DECLARATIONS_

void UbiArduino_MKR_GSM::init()
{
    
}

UbiArduino_MKR_GSM::UbiArduino_MKR_GSM()
{
    _userAgent = const_cast<char *>(MKRGSM_USER_AGENT);
}

UbiArduino_MKR_GSM::~UbiArduino_MKR_GSM()
{
    delete UbiDevice::_device;
}

UbiDevice *UbiArduino_MKR_GSM::getDeviceInstance()
{
    if (UbiDevice::_device == nullptr)
    {
        UbiDevice::_device = static_cast<UbiDevice *>(new UbiArduino_MKR_GSM);
    }
    return UbiDevice::_device;
}

bool UbiArduino_MKR_GSM::connect()
{
    bool Connected{false};
    uint32_t reconnectionAttempt = 0;
    _APN = _credentials[0];
    _login = _credentials[1];
    _password = _credentials[2];
    _pin = _credentials[3];

    while (!Connected && reconnectionAttempt < _maxConnectionRetries)
    {
        if (_debug)
        {
            Serial.print("Connecting to URL: ");
            Serial.println(_host);
            Serial.print("On port: ");
            Serial.println(_port);
            Serial.print("Attempt #: ");
            Serial.print(reconnectionAttempt + 1);
            Serial.println("");
        }

        if (_gsm.begin(_pin) == GSM_READY)
        {
            if (_debug)
            {
                Serial.println("[1/3] SIM card ready!");
                Serial.println("Attaching to GPRS network");
            }
            if (_gprs.attachGPRS(_APN, _login, _password) == GPRS_READY)
            {
                if (_debug)
                    Serial.println("[2/3] GPRS ready!");
                if (_client.connect(_host, _port) == true)
                {
                    if (_debug)
                        Serial.println("[3/3] Ubidots connection ready!");
                    Connected = true;
                }
            }
        }
        reconnectionAttempt++;
    }
    //strcmp is equal to 0 when both strings are identical
    //if module is connected and uniqueID is "undefined" call the getUniqueID method
    //this is to avoid calling the method again the the uniqueID has already been set
    if (Connected && !strcmp(_uniqueID, "undefined"))
        this->getUniqueID(this->_uniqueID);

    return Connected;
}
bool UbiArduino_MKR_GSM::reconnect()
{
    return connect();
}
bool UbiArduino_MKR_GSM::serverConnected()
{
    return false;
}

bool UbiArduino_MKR_GSM::connected()
{
    return _client.connected();
}

bool UbiArduino_MKR_GSM::sendHttp(const char *deviceLabel, const char *deviceName, const char *payload)
{
    bool retVal{false};
    if (_client.connected())
    {
        if (_debug)
        {
            Serial.print("sending through http to device with label: ");
            Serial.println(deviceLabel);
        }

        _client.print(F("POST /api/v1.6/devices/"));
        _client.print(deviceLabel);
        _client.println(F(" HTTP/1.1"));
        _client.print(F("Host: "));
        _client.println(_host);
        _client.println(F("User-Agent: Soracom/1.0"));
        _client.println(F("Content-type: application/json"));
        _client.print(F("X-Auth-Token: "));
        _client.println(F(_token));

        _client.print(F("Content-Length: "));
        _client.println(strlen(payload));

        _client.println();
        _client.println(payload);

        while (!_client.available())
            ;

        if (_client.ready() == 0)
        {
            Serial.print(F("."));
            delay(50);
        }
        if (_debug)
        {
            while (_client.available())
            {
                char c = _client.read();
                Serial.print(c);
            }
        }
        Serial.println("Data sent succesfully to: ");
        Serial.println(_host);
        retVal = true;
    }
    else
    {
        if (_debug)
            Serial.println("Not connected, trying to reconnect...");
        if (_client.connect(_host, _port))
        {
            if (_debug)
            {
                Serial.println("reconnect succesful");
                Serial.println("Sending data: ");
            }
            retVal = sendHttp(deviceLabel, deviceName, payload);
        }
        else
        {
            if (_debug)
                Serial.println("reconnect failed");
            retVal = false;
        }
    }
    return retVal;
}
bool UbiArduino_MKR_GSM::sendTcp(const char *deviceLabel, const char *deviceName, const char *payload)
{
    if (!_client.connected())
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
    _client.print("");
    _client.print(payload);

    if (!UbiUtils::waitServerAnswer(_client, _debug))
    {
        if (_debug)
        {
            Serial.println("[ERROR] Could not read server's response");
        }
        _client.flush();
        _client.stop();
        return false;
    }

    /* Parses the host answer, returns true if it is 'Ok' */
    char *response = (char *)malloc(sizeof(char) * 100);
    float value = UbiUtils::parseTCPAnswer("POST", response, _client, _debug);
    free(response);
    if (value != ERROR_VALUE)
    {
        _client.flush();
        _client.stop();
        return true;
    }
    _client.flush();
    _client.stop();
    return false;
}
bool UbiArduino_MKR_GSM::sendUdp(const char *deviceLabel, const char *deviceName, const char *payload)
{
    //unsigned int localPort{2390};
    unsigned int localPort{UBIDOTS_TCP_PORT};

    if (!connected())
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

    if (_debug)
        Serial.println("\nStarting connection to server...");

    _udp.begin(localPort);
    if (!(_udp.beginPacket(_host, _port) && _udp.write(payload, strlen(payload)) && _udp.endPacket()))
    {
        if (_debug)
            Serial.println("ERROR sending values with UDP");
        _udp.stop();
        return false;
    }
    _udp.stop();
    return true;
}

double UbiArduino_MKR_GSM::getTcp(const char *deviceLabel, const char *variableLabel)
{
    if (!connected())
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
            return ERROR_VALUE;
        }
    }
    _client.print(_userAgent);
    _client.print("|LV|");
    _client.print(_token);
    _client.print("|");
    _client.print(deviceLabel);
    _client.print(":");
    _client.print(variableLabel);
    _client.print("|end");

    if (_debug)
    {
        Serial.println("----");
        Serial.println("Payload for request:");
        Serial.print(_userAgent);
        Serial.print("|LV|");
        Serial.print(_token);
        Serial.print("|");
        Serial.print(deviceLabel);
        Serial.print(":");
        Serial.print(variableLabel);
        Serial.print("|end");
        Serial.println("\n----");
    }
    if (!UbiUtils::waitServerAnswer(_client, _debug))
    {
        return ERROR_VALUE;
    }

    char *response = (char *)malloc(sizeof(char) * 100);
    float value = UbiUtils::parseTCPAnswer("LV", response, _client, _debug);
    _client.flush();
    _client.stop();
    return value;
}

void UbiArduino_MKR_GSM::getUniqueID(char *ID)
{
    if (_debug)
        Serial.println("getting IMEI");
    if (strcmp(_uniqueID, "undefined") == 0)
    {
        _modem.getIMEI().toCharArray(_uniqueID, 16);
    }
    ID = _uniqueID;
    if (_debug)
    {
        Serial.println("Done! IMEI is: ");
        Serial.println(ID);
    }
}

double UbiArduino_MKR_GSM::getHttp(const char *device_label, const char *variable_label)
{
    bool connected = connected();
    if (!connected)
    {
        Serial.println("not connected, returning an error :");
        return ERROR_VALUE;
    }
    if (_debug)
    {
        Serial.println("\nGetting data from: ");
        Serial.print(_host);
        Serial.print("\n");
    }
    uint16_t pathLength = UbiUtils::getPathLength(device_label, variable_label);
    char *path = (char *)malloc(sizeof(char) * pathLength + 1);
    sprintf(path, "/api/v1.6/devices/%s/%s/lv", device_label, variable_label);
    if (_debug)
    {
        Serial.print(F("\nRequesting to URL: "));
        Serial.println(path);
    }

    uint16_t requestLineLength = UbiUtils::getRequestLength(path, _host, _token, _userAgent);
    char *message = (char *)malloc(sizeof(char) * requestLineLength + 1);
    sprintf(message,
            "GET %s HTTP/1.1\r\nHost: %s\r\nX-Auth-Token: "
            "%s\r\nUser-Agent: %s\r\nContent-Type: "
            "application/json\r\nConnection: close\r\n\r\n",
            path, _host, _token, _userAgent);

    if (_debug)
    {
        Serial.println(F("Request sent"));
        Serial.println(message);
    }
    _client.print(message);
    Serial.println(message);

    while (_client.connected())
    {
        if (_client.available())
        {
            String line = _client.readStringUntil('\n');
            if (line == "\r")
            {
                break;
            }
        }
    }

    free(message);
    free(path);
    double value = UbiUtils::parseServerAnswer(_client, _debug);

    _client.flush();
    _client.stop();
    return value;
}

#endif

#endif