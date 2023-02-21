#include "Ubidots.h"

Ubidots::Ubidots(const char *token, const IotProtocol &iotProtocol, const char *host)
{
	_deviceSettings._token = const_cast<char *>(token);
	_deviceSettings._port = getPort(iotProtocol);
	_deviceSettings._host = const_cast<char *>(host);
	_deviceSettings._protocol = iotProtocol;
	_deviceSettings._credentials = nullptr;
	device->setSettings(_deviceSettings);
	device->buildProtocolHandlerInstance(iotProtocol);
	device->_protocolHandler->init();
	//device->getUniqueID(_defaultDeviceLabel);
	// Given the fact that the "device" object is instantiated before calling "Ubidots" constructor, it is not possible to assing
	// to the protocol pointer at the "device" constructor, because at that point, host, token and protocol are not known
}

Ubidots::~Ubidots()
{
}

void Ubidots::add(const char *variableLabel, double value)
{
	add(variableLabel, value, NULL, NULL, NULL);
}

void Ubidots::add(const char *variableLabel, double value, char *context)
{
	add(variableLabel, value, context, NULL, NULL);
}

void Ubidots::add(const char *variableLabel, double value, char *context, long unsigned dotTimestampSeconds)
{
	add(variableLabel, value, context, dotTimestampSeconds, NULL);
}

void Ubidots::add(const char *variableLabel, double value, char *context, long unsigned dotTimestampSeconds, unsigned int dotTimestampMillis)
{
	device->_protocolHandler->add(variableLabel, value, context, dotTimestampSeconds, dotTimestampMillis);
}

bool Ubidots::send() { return send(_defaultDeviceLabel, _defaultDeviceLabel); }

bool Ubidots::send(const char *device_label) { return send(device_label, device_label); }

bool Ubidots::send(const char *device_label, const char *device_name)
{
	if (strlen(_deviceType) > 0 && _deviceSettings._protocol == UBI_HTTP)
	{
		char builtDeviceLabel[50];
		sprintf(builtDeviceLabel, "%s/?type=%s", device_label, _deviceType);
		return device->_protocolHandler->send(builtDeviceLabel, device_name);
	}
	return device->_protocolHandler->send(device_label, device_name);
}

bool Ubidots::serverConnected()
{
	return device->_protocolHandler->serverConnected();
}

bool Ubidots::connected()
{
	return device->_protocolHandler->connected();
}
unsigned int Ubidots::getPort(const IotProtocol &protocol)
{
	uint32_t port = 0;
	switch (protocol)
	{
	case UBI_HTTP:
		port = UBIDOTS_HTTP_PORT;
		break;
	case UBI_TCP:
		port = UBIDOTS_TCP_PORT;
		break;
	case UBI_UDP:
		port = UBIDOTS_TCP_PORT;
		break;
	case UBI_HTTPS:
		port = UBIDOTS_HTTPS_PORT;
		break;
	case UBI_TCPS:
		port = UBIDOTS_TCPS_PORT;
		break;
	}
	return port;
}

bool Ubidots::connect(const char *ssid, const char *password)
{
	const char *credentials[2]{ssid, password};
	_deviceSettings._credentials = const_cast<char **>(credentials);
	device->setSettings(_deviceSettings);
	return device->_protocolHandler->connect();
}

// TODO implement as a functor class
bool Ubidots::connect(const char *APN, const char *username, const char *password, const char *pin)
{
	const char *credentials[4]{APN, username, password, pin};
	_deviceSettings._credentials = const_cast<char **>(credentials);
	device->setSettings(_deviceSettings);
	return device->_protocolHandler->connect();
}

double Ubidots::get(const char *device_label, const char *variable_label)
{
	return device->_protocolHandler->get(device_label, variable_label);
}

void Ubidots::addContext(char *key_label, char *key_value)
{
	device->_protocolHandler->addContext(key_label, key_value);
}

void Ubidots::getContext(char *context_result)
{
	device->_protocolHandler->getContext(context_result);
}

void Ubidots::setDebug(const bool &debug)
{
	this->_debug = debug;
	device->_protocolHandler->setDebug(debug);
}

void Ubidots::setDeviceType(const char *deviceType)
{
	if (strlen(deviceType) > 0 && _deviceSettings._protocol == UBI_HTTP)
	{
		sprintf(_deviceType, "%s", deviceType);
	}
	else
	{
		Serial.println("Device Type is only available using HTTP");
	}
}

void Ubidots::getDeviceID(char *ID)
{
	device->getUniqueID(ID);
}

bool Ubidots::reconnect()
{
	return device->_protocolHandler->reconnect();
}