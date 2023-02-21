#ifndef _UBI_DEVICE_H_
#define _UBI_DEVICE_H_

#include "Arduino.h"
#include "UbiTypes.h"
#include "UbiConstants.h"
#include "UbiProtocolHandler.h"

class UbiDevice;
extern UbiDevice *device;

class UbiDevice
{
public:
    virtual ~UbiDevice();
    void setDebug(const bool &debug);
    void setSettings(const UbiDeviceSettings &arg);
    void buildProtocolHandlerInstance(const IotProtocol &protocol);
    virtual void getUniqueID(char *ID) = 0;

    // Assign the user agent as undefined by default. This forces to instantiate
    userAgent _userAgent{"undefined"};
    char _uniqueID[18]{"undefined"};

    UbiProtocolHandler *_protocolHandler{nullptr};

protected:
    // Constructor needs to be protected in order for the derived classs to be able to call the Base class constructor
    explicit UbiDevice();
    /*static pointer to store this class's singleton instance*/
    /*needs to be protected in order for the derived class's "constructor" to be able to invoke this*/
    inline static UbiDevice *_device{nullptr};

    bool _debug{false};

    // All of the derived "devices" class need to use these members. So they need to be declared as protected
    ubiToken _token;
    hostUrl _host;
    portNumber _port;
    IotProtocol _protocol;
    char **_credentials;

private:
    virtual bool connect() = 0;
    virtual bool reconnect() = 0;
    virtual bool serverConnected() = 0;
    virtual bool connected() = 0;
    virtual void init() = 0;
    virtual bool sendHttp(const char *deviceLabel, const char *deviceName, const char *payload) = 0;
    virtual bool sendTcp(const char *deviceLabel, const char *deviceName, const char *payload) = 0;
    virtual bool sendUdp(const char *deviceLabel, const char *deviceName, const char *payload) = 0;
    virtual double getHttp(const char *deviceLabel, const char *variableLabel) = 0;
    virtual double getTcp(const char *deviceLabel, const char *variableLabel) = 0;

    friend class ProtocolHandler<UBI_HTTP>;
    friend class ProtocolHandler<UBI_TCP>;
    friend class ProtocolHandler<UBI_UDP>;
    friend class UbiProtocolHandler;
};

#ifndef _PROTOCOL_ONLY_DEFINITIONS_

/* <-------------------------------------------------------------------------------------------> */
/* <----------------------------------Add functions--------------------------------------------> */
/* <-------------------------------------------------------------------------------------------> */

template <IotProtocol Protocol>
void ProtocolHandler<Protocol>::add(const char *variableLabel, double value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis)
{
    _dirty = true;
    (_dots + _currentValue)->variable_label = variableLabel;
    (_dots + _currentValue)->dot_value = value;
    (_dots + _currentValue)->dot_context = context;
    (_dots + _currentValue)->dot_timestamp_seconds = dot_timestamp_seconds;
    (_dots + _currentValue)->dot_timestamp_millis = dot_timestamp_millis;
    _currentValue++;
    if (_currentValue > MAX_VALUES)
    {
        if (_debug)
        {
            Serial.println(F("You are sending more than the maximum of consecutive variables"));
        }
        _currentValue = MAX_VALUES;
    }
}
/* <-------------------------------------------------------------------------------------------> */
/* <---------------------------------Send functions--------------------------------------------> */
/* <-------------------------------------------------------------------------------------------> */
template <IotProtocol Protocol>
bool ProtocolHandler<Protocol>::send(const char *deviceLabel, const char *deviceName)
{
    char* payload = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    
    if constexpr(Protocol ==  UBI_HTTP || Protocol == UBI_HTTPS)
    {
        UbiUtils::buildHttpPayload(payload, _currentValue, _dots, _debug);
    }
    else if constexpr(Protocol == UBI_TCP || Protocol == UBI_TCPS || Protocol == UBI_UDP)
    {
        UbiUtils::buildTcpPayload(payload, device->_token, deviceLabel, deviceName, device->_userAgent, _currentValue, _dots,_debug);
    }

    if (_debug)
        Serial.println("Sending data...");

    bool result{false};
    
    if constexpr(Protocol ==  UBI_HTTP || Protocol == UBI_HTTPS)
    {
        result = device->sendHttp(deviceLabel, deviceName, payload);
    }
    else if constexpr(Protocol == UBI_TCP || Protocol == UBI_TCPS)
    {
        result = device->sendTcp(deviceLabel, deviceName, payload);
    }
    else if constexpr(Protocol == UBI_UDP)
    {
        result = device->sendUdp(deviceLabel, deviceName, payload);
    }

    free(payload);
    if(result) 
    {
        _dirty = false;
        _currentValue = 0;
    }
    _currentValue = 0;
    return result;
}


/* <-------------------------------------------------------------------------------------------> */
/* <---------------------------------Get functions--------------------------------------------> */
/* <-------------------------------------------------------------------------------------------> */
template <IotProtocol Protocol>
double ProtocolHandler<Protocol>::get(const char *device_label, const char *variable_label)
{
    double value{ERROR_VALUE};
    if constexpr(Protocol ==  UBI_HTTP || Protocol == UBI_HTTPS)
        value = device->getHttp(device_label, variable_label);
    if constexpr(Protocol ==  UBI_TCP || Protocol == UBI_TCPS)
        value = device->getTcp(device_label, variable_label);
    if constexpr(Protocol ==  UBI_UDP)
        Serial.println("ERROR, data retrieval is only supported using TCP or HTTP protocols");
    return value;
}

/* <-------------------------------------------------------------------------------------------> */
/* <---------------------------------GetContext functions--------------------------------------------> */
/* <-------------------------------------------------------------------------------------------> */
template <IotProtocol Protocol>
void ProtocolHandler<Protocol>::getContext(char *context_result)
{
    sprintf(context_result, "");
    for (uint8_t i = 0; i < _currentContext; )
    {
        if constexpr(Protocol == UBI_TCP || Protocol == UBI_UDP)
            sprintf(context_result, "%s%s=%s", context_result, (_context + i)->key_label, (_context + i)->key_value);
        else if constexpr(Protocol == UBI_HTTP)
            sprintf(context_result, "%s\"%s\":\"%s\"", context_result, (_context + i)->key_label, (_context + i)->key_value);

        i++;

        if (i < _currentContext)
        {
            if constexpr(Protocol == UBI_TCP || Protocol == UBI_UDP)
                sprintf(context_result, "%s$", context_result);
            else if constexpr(Protocol == UBI_HTTP)
                sprintf(context_result, "%s,", context_result);
        }
        else
        {
            sprintf(context_result, "%s", context_result);
            _currentContext = 0;
        }
    }
}


#endif

#endif