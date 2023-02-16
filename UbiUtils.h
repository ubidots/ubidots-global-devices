#ifndef __UBIUTILS_H__
#define __UBIUTILS_H__

#include <cstdlib>
#include "Arduino.h"
#include "UbiConstants.h"
#include <string.h>
#include <functional>

class UbiUtils
{
public:
    template <typename Callable>
    static double parseServerAnswer(Callable &client, const bool &debug);
    template <typename Callable>
    static bool waitServerAnswer(Callable &client, const bool &debug);
    template <typename Callable>
    static float parseTCPAnswer(const char *requestType, char *response, Callable client, const bool &debug);
    template <typename Callable>
    static void parsePartialServerAnswer(char *serverResponse, Callable &client, const bool &debug);
    template <typename Callable, typename SyncronizeTime>
    static bool preConnectionChecks(Callable &client, unsigned long &timerToSync, SyncronizeTime& syncronizeTime, const bool &debug = false);
    template <typename Callable>
    static void loadCerts(Callable &client, const unsigned char cert1[], const unsigned int& size1, const unsigned char cert2[], const unsigned int& size2, const bool &debug = false);
    static bool syncronizeTime(const bool &debug = false);

    static int hexadecimalToDecimal(char hexVal[]);
    static void buildTcpPayload(char *payload, const char *token, const char *deviceLabel, const char *deviceName, const char *userAgent, int8_t &currentValue, Value *dots, const bool &debug);
    static void buildHttpPayload(char *payload, int8_t &currentValue, Value *dots, const bool &debug);

    static void floatToChar(char *strValue, float value);
    static uint16_t getPathLength(const char *deviceLabel, const char *variableLabel);
    static uint16_t getRequestLength(const char *path, const char *host, const char *token, const char *userAgent);
};

template <typename Callable>
double UbiUtils::parseServerAnswer(Callable &client, const bool &debug)
{

    /**
     * @param _charResponseLength 3 is the maximun amount of digits for the length
     * to have
     */
    char *_charLength = (char *)malloc(sizeof(char) * 3);

    UbiUtils::parsePartialServerAnswer(_charLength, client, debug);
    /**
     * The server respond the length of the value in HEX so it has to be converted
     * to DEC
     * */
    uint8_t length = UbiUtils::hexadecimalToDecimal(_charLength);
    char *_charValue = (char *)malloc(sizeof(char) * length + 1);

    parsePartialServerAnswer(_charValue, client, debug);

    double value = strtof(_charValue, NULL);

    free(_charLength);
    free(_charValue);

    Serial.println(value);
    return value;
}

template <typename Callable>
bool UbiUtils::waitServerAnswer(Callable &client, const bool &debug)
{
    int timeout = 0;
    while (!client.available() && timeout < 50000)
    {
        timeout++;
        delay(1);
        if (timeout > 50000 - 1)
        {
            if (debug)
                Serial.println("timeout, could not read any response from the host");

            client.flush();
            client.stop();
            return false;
        }
    }
    return true;
}

template <typename Callable>
float UbiUtils::parseTCPAnswer(const char *requestType, char *response, Callable client, const bool &debug)
{
    int j = 0;

    if (debug)
    {
        Serial.println(F("----------"));
        Serial.println(F("Server's response:"));
    }

    while (client.available())
    {
        char c = client.read();
        if (debug)
        {
            Serial.write(c);
        }
        response[j] = c;
        j++;
        if (j >= MAX_BUFFER_SIZE - 1)
        {
            break;
        }
    }

    if (debug)
    {
        Serial.println(F("\n----------"));
    }

    response[j] = '\0';
    float result = ERROR_VALUE;

    // POST
    if (requestType == "POST")
    {
        char *pch = strstr(response, "OK");
        if (pch != NULL)
        {
            result = 1;
        }
        return result;
    }

    // LV
    char *pch = strchr(response, '|');
    if (pch != NULL)
    {
        result = atof(pch + 1);
    }

    return result;
}

template <typename Callable>
void UbiUtils::parsePartialServerAnswer(char *serverResponse, Callable &client, const bool &debug)
{
    /**
    * Server Response Ascii code -> Character from the server
    First extract the following value
    52 -> 4     ->Length of the value in HEX
    13 -> \r
    10 -> \n

    At the next time it will read the whole value with the allocated memory set by the previus value extracted
    51 -> 3     ->Value in char
    57 -> 9     ->Value in char
    46 -> .     ->Decimal point
    48 -> 0     ->Value in char
    13 -> \r
    10 -> \n

    48 -> 0     ->Value in char
    13 -> \r
    10 -> \n

    13 -> \r
    10 -> \n
    */
    sprintf(serverResponse, "%c", client.read());

    while (client.available())
    {
        char charRead = (char)client.read();
        char c[2];
        sprintf(c, "%c\0", charRead);
        if (charRead == '\r')
        { // If the character is \r means we have ended the line then we request
            // Get the last character \n to enable the function to run again
            client.read(); // clean the buffer asking for the next character
            break;
        }
        else if (charRead == 'e')
        {
            /**
             * After 18 digits it will show the response in scientific notation, and
             * there is no space to store such a  huge number
             * */
            sprintf(serverResponse, "%f", ERROR_VALUE);
            if (debug)
            {
                Serial.println(F("[ERROR]The value from the server exceeded memory capacity"));
            }
        }
        else
        {
            strcat(serverResponse, c); // Add the value to the expected response.
        }
    }
}

template <typename Callable, typename SyncronizeTime>
bool UbiUtils::preConnectionChecks(Callable &certs, unsigned long &timerToSync, SyncronizeTime& syncronizeTime, const bool &debug)
{
    bool syncronized = true;
    if (millis() - timerToSync > 3600000)
    {
        syncronized = syncronizeTime(debug);
        timerToSync = millis();
    }

    if (!syncronized)
    {
        if (debug)
        {
            Serial.println(
                F("[ERROR] Could not syncronize device time with external "
                  "source, make sure that you are not behind a "
                  "firewall"));
        }
        return false;
    }

    if (!certs.getCount())
    {
        if (debug)
        {
            Serial.println(F("[ERROR] Please load a valid certificate"));
        }
        return false;
    }

    return true;
}

template <typename Callable>
void UbiUtils::loadCerts(Callable &client, const unsigned char cert1[], const unsigned int& size1, const unsigned char cert2[], const unsigned int& size2, const bool &debug)
{
    client.append(cert1, size1);
    client.append(cert2, size2);
    if (!client.getCount() && debug)
    {
        Serial.println("Failed to load root CA certificates!");
    }
}


#endif // __UBIUTILS_H__