#include "UbiDevice.h"




UbiDevice::UbiDevice() 
{ 
    
}

UbiDevice::~UbiDevice()
{
	delete _protocolHandler;
	delete _device;
}


void UbiDevice::setSettings(const UbiDeviceSettings &arg)
{ 
    this->_credentials = arg._credentials;
    this->_host = arg._host;
    this->_token = arg._token;
    this->_port = arg._port;
    this->_protocol = arg._protocol;
}

void UbiDevice::buildProtocolHandlerInstance(const IotProtocol& protocol)
{
	if(protocol == UBI_HTTP || protocol == UBI_HTTPS)
   		this->_protocolHandler = reinterpret_cast<UbiProtocolHandler*>(new ProtocolHandler<UBI_HTTP>(protocol));
	else if(protocol == UBI_TCP)
   		this->_protocolHandler = reinterpret_cast<UbiProtocolHandler*>(new ProtocolHandler<UBI_TCP>(protocol));
	else if(protocol == UBI_UDP)
   		this->_protocolHandler = reinterpret_cast<UbiProtocolHandler*>(new ProtocolHandler<UBI_UDP>(protocol));

}

void UbiDevice::setDebug(const bool &debug) { _debug = debug; }











void UbiProtocolHandler::setDebug(const bool& debug)
{
	this->_debug = debug;
	device->setDebug(debug);
}

bool UbiProtocolHandler::connect()
{
	return device->connect();
}

bool UbiProtocolHandler::reconnect()
{
   return device->reconnect(); 
}

bool UbiProtocolHandler::serverConnected()
{
   return device->serverConnected(); 
}


void UbiProtocolHandler::addContext(char* key_label, char* key_value)
{
	(_context + _currentContext)->key_label = key_label;
	(_context + _currentContext)->key_value = key_value;
	_currentContext++;
	if (_currentContext >= MAX_VALUES)
	{
		Serial.println(F("You are adding more than the maximum of consecutive "
						 "key-values pairs"));
		_currentContext = MAX_VALUES;
	}
}

void UbiProtocolHandler::init()
{
	device->init();
}