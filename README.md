# Ubidots global library

This library provides an easy way to connect several different devices to Ubidots by creating a common interface despite all the hardware differences. 

Currently the library is implemented for:
 - [Arduino](https://store-usa.arduino.cc/products/arduino-mkr-gsm-1400).
 - [ESP8266](https://www.espressif.com/en/products/socs/esp8266)


## Requirements

 - [Arduino IDE](https://wiki-content.arduino.cc/en/software), [PlatformIO](https://platformio.org/) or any IDE of your preference
 - [Ubidots Global Devices Library](https://github.com/ubidots/ubidots-global-devices)

 ## Setup

1.Depending if you are using PlatformIO or Arduino IDE, you might need to install the specific library dependencies according to your Development board. For example:

PlatformIO: if you are using the Arduino MKR GSM 1400 you need to edit the "platformio.ini" file in your project to add the MKRGSM library in the following way:
 
    lib_deps = arduino-libraries/MKRGSM@^1.5.0

Arduino: if you are using the Arduino MKR GSM 1400 you need to install the corresponding Arduino SAMD Boards dependencies from the Boards Manager menu.lib_deps = arduino-libraries/MKRGSM@^

2.Download the **Ubidots Global Devices Library** [here](https://github.com/ubidots/ubidots-global-devices).
Add the library to your development environment. This may vary depending on which are you using. For example:

PlatformIO: unzip the library and add it to your project's **lib** directory

Arduino IDE: click on **Sketch -> Include Library -> Add .Zip Lirary** then select the Ubidots .ZIP file and click **Accept**.
Close the Arduino IDE and open it again.

# Documentation

## Constructor

### Ubidots

`Ubidots(const char *token, const IotProtocol& iotProtocol, const char* host)`

Creates an Ubidots instance with which the user interacts with.
 - @token,[Required]. Your Ubidots unique account [TOKEN](http://help.ubidots.com/user-guides/find-your-token-from-your-ubidots-account).
 - @iotProtocol, [Optional]. [Options] = [`UBI_HTTP`, `UBI_TCP`, `UBI_UDP`, `UBI_HTTPS`], [Default] = `UBI_HTTP`. The IoT protocol that you will use to send or retrieve data.
 - @host, [Optional]. [Options] = [`UBI_INDUSTRIAL`, `<custom_server>`], [Default] = `UBI_INDUSTRIAL`. The server to send data, set `<custom_server>` if you wish to point to another private server given by Ubidots.

 **NOTE:** If you use HTTPS for the devices which support it, the client will implement TLS 2.0 based on the [example for ESP32 secure client](https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/BasicHttpsClient/BasicHttpsClient.ino) by Espressif, to secure your data. Keep in mind that due to the security checks needed, the packet may take a little more time to be sent than without TLS. If you wish to send data insecurely, use UDP or any of the available examples at our [docs](https://ubidots.com/docs/hw/).

As Ubidots makes its best to secure your data, we do not guarantee any issue, data miss or external sniff coming from the native secure client or bugs in the library.

## Methods

`void add(const char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis)`

Adds a dot with its related value, context and timestamp to be sent to a certain data source.

**Important:** The max payload length is 700 bytes, if your payload is greater it won't be properly sent. You can see on your serial console the payload to send if you call the `setDebug(bool debug)` method and pass a true value to it.

- @variable_label, [Required]. The label of the variable where the dot will be stored.
- @value, [Required]. The value of the dot.
- @context, [Optional]. The dot's context.
- @dot_timestamp_seconds, [Optional]. The dot's timestamp in seconds.
- @dot_timestamp_millis, [Optional]. The dot's timestamp number of milliseconds. If the timestamp's milliseconds values is not set, the seconds will be multplied by 1000.

`float get(const char* device_label, const char* variable_label)`

Returns as float the last value of the dot from the variable.

- @device_label, [Required]. The device label which contains the variable to retrieve values from.
- @variable_label, [Required]. The variable label to retrieve values from.

`void addContext(char *key_label, char *key_value)`

Adds to local memory a new key-value context key. The method inputs must be char pointers. The method allows to store up to 10 key-value pairs.

- @key_label, [Required]. The key context label to store values.
- @key_value, [Required]. The key pair value.

`void getContext(char *context)`

Builds the context according to the chosen protocol and stores it in the context char pointer.

- @context, [Required]. A char pointer where the context will be stored.

`void setDebug(bool debug)`

Makes available debug messages through the serial port.

- @debug, [Required]. Boolean type to turn off/on debug messages.

`bool send(const char* device_label, const char* device_name);`

Sends all the data added using the add() method. Returns true if the data was sent.

- @device_label, [Optional], [Default] = Device's MAC Address. The device label to send data. If not set, the device's MAC address will be used.
- @device_name, [Optional], [Default] = @device_label. The device name otherwise assigned if the device doesn't already exist in your Ubidots account. If not set, the device label parameter will be used. **NOTE**: Device name is only supported through TCP/UDP, if you use another protocol, the device name will be the same as device label.

`bool connect(const char* ssid, const char* password)`

Attempts to connect to the cloud using WiFi with the specified credentials.

- @ssid, [Required]. WiFi SSID to connect to name.
- @password, [Required]. WiFi password credential.

`bool connect(const char* apn, const char* username, const char* password, const char* pin)`

Attempts to connect to the cloud using GSM with the specified credentials.

- @apn, [Required]. Your cellular carrier APN.
- @username, [Required]. Your cellular carrier username.
- @password, [Required]. Your cellular carrier password.
- @pin, [optional]. Your sim Card pin.

`void setDeviceType(const char* deviceType)`

Sets a [device type](https://help.ubidots.com/en/articles/2129204-device-types) to be added in your request. This method works only if you set HTTP as iot protocol in your instance constructor.


# Examples

Refer to the [examples](/examples) folder
