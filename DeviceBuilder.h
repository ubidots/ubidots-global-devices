#ifndef _DEVICE_BUILDER_H_
#define _DEVICE_BUILDER_H_

//#define _PROTOCOL_ONLY_DEFINITIONS_
#include "UbiDevice.h"


//TODO: refactor such that there is no need to define "getDeviceInstance" method for each device
// The only possible way to avoid having to define "getDeviceInstance" method for each device
// is to use c++20 Concepts which allow to define a template based upon structural typing 
// however it is not yet supported by Arduino IDE compiler natively


#if defined(MKR_1400_GSM)
    #include "Devices/UbiArduino_MKR_GSM.h"
    UbiDevice *device = UbiArduino_MKR_GSM::getDeviceInstance();
#elif defined(ESP32_CAM)
    #include "Devices/UbiESP32.h"
    UbiDevice *device = UbiESP32::getDeviceInstance();
#elif defined(_UBI_ESP8266_)
    #include "Devices/UbiESP8266.h"
    UbiDevice* device = UbiESP8266::getDeviceInstance();
#else

#define _ONLY_DECLARATIONS_


//ifdefined endif
#endif



//Guard header endif
#endif