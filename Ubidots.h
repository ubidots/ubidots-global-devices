#ifndef __UBIDOTS_H_59QTAT1NY8HK__
#define __UBIDOTS_H_59QTAT1NY8HK__

///////////////////////////////////////////////////////////
/// @brief Ubidots
///
#include "DeviceBuilder.h"


class Ubidots
{
public:

    explicit Ubidots(const char *token, const IotProtocol& iotProtocol = UBI_HTTP, const char* host = UBI_INDUSTRIAL);
  	~Ubidots();
  	bool connect(const char *APN, const char *username, const char *password, const char *pin);
  	bool connect(const char *ssid, const char *password);
	bool reconnect();
  	bool send();
  	bool send(const char* deviceLabel);
  	bool send(const char* deviceLabel, const char* deviceName);
  	bool serverConnected();
	bool connected();
  	double get(const char *deviceLabel, const char *variableLabel);
  	void add(const char *variableLabel, double value, char *context, unsigned long dotTimestampSeconds, unsigned int dotTimestampSecondsMillis);
  	void add(const char *variableLabel, double value, char *context, unsigned long dotTimestampSeconds);
  	void add(const char *variableLabel, double value, char *context);
  	void add(const char *variableLabel, double value);
  	void addContext(char *keyLabel, char *keyValue);
  	void getContext(char *context_result);
  	void setDebug(const bool& debug);
  	void setDeviceType(const char *deviceType);
  	void getDeviceID(char *ID);



private:
    // Members
    bool _debug{false};
    char _defaultDeviceLabel[18]{"undefined"};
    char _deviceType[25]{"undefined"};
    UbiDeviceSettings _deviceSettings;

	unsigned int getPort(const IotProtocol& protocol);
};

#endif  // __UBIDOTS_H_59QTAT1NY8HK__
