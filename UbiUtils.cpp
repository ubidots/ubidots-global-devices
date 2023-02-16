#include "UbiUtils.h"

int UbiUtils::hexadecimalToDecimal(char hexVal[])
{
	int len = strlen(hexVal);

	// Initializing base value to 1, i.e 16^0
	int base = 1;

	int dec_val = 0;

	// Extracting characters as digits from last character
	for (int i = len - 1; i >= 0; i--)
	{
		// if character lies in '0'-'9', converting
		// it to integral 0-9 by subtracting 48 from
		// ASCII value.
		if (hexVal[i] >= '0' && hexVal[i] <= '9')
		{
			dec_val += (hexVal[i] - 48) * base;
			// incrementing base by power
			base = base * 16;
		}

		// if character lies in 'A'-'F' , converting
		// it to integral 10 - 15 by subtracting 55
		// from ASCII value
		else if (hexVal[i] >= 'A' && hexVal[i] <= 'F')
		{
			dec_val += (hexVal[i] - 55) * base;
			// incrementing base by power
			base = base * 16;
		}
	}
	return dec_val;
}

void UbiUtils::buildTcpPayload(char *payload, const char *token, const char *deviceLabel, const char *deviceName, const char *userAgent, int8_t &currentValue, Value *dots, const bool &debug)
{
	sprintf(payload, "");
	sprintf(payload, "%s|POST|%s|", userAgent, token);
	sprintf(payload, "%s%s:%s", payload, deviceLabel, deviceName);
	sprintf(payload, "%s=>", payload);
	for (uint8_t i = 0; i < currentValue;)
	{
		char str_value[20];
		floatToChar(str_value, (dots + i)->dot_value);
		sprintf(payload, "%s%s:%s", payload, (dots + i)->variable_label, str_value);

		// Adds dot context
		if ((dots + i)->dot_context != NULL)
		{
			sprintf(payload, "%s$%s", payload, (dots + i)->dot_context);
		}

		// Adds timestamp seconds
		if ((dots + i)->dot_timestamp_seconds != NULL)
		{
			sprintf(payload, "%s@%lu", payload, (dots + i)->dot_timestamp_seconds);
			// Adds timestamp milliseconds
			if ((dots + i)->dot_timestamp_millis != NULL)
			{
				char milliseconds[3];
				int timestamp_millis = (dots + i)->dot_timestamp_millis;
				uint8_t units = timestamp_millis % 10;
				uint8_t dec = (timestamp_millis / 10) % 10;
				uint8_t hund = (timestamp_millis / 100) % 10;
				sprintf(milliseconds, "%d%d%d", hund, dec, units);
				sprintf(payload, "%s%s", payload, milliseconds);
			}
			else
			{
				sprintf(payload, "%s000", payload);
			}
		}

		i++;

		if (i < currentValue)
		{
			sprintf(payload, "%s,", payload);
		}
		else
		{
			sprintf(payload, "%s|end", payload);
			currentValue = 0;
		}
	}

	if (debug)
	{
		Serial.println("----------");
		Serial.println("payload:");
		Serial.println(payload);
		Serial.println("----------");
		Serial.println("");
	}
}

void UbiUtils::buildHttpPayload(char *payload, int8_t &currentValue, Value *dots, const bool &debug)
{
	/* Builds the payload */
	sprintf(payload, "{");

	for (uint8_t i = 0; i < currentValue;)
	{
		char str_value[25];
		floatToChar(str_value, (dots + i)->dot_value);
		sprintf(payload, "%s\"%s\":{\"value\":%s", payload, (dots + i)->variable_label, str_value);

		// Adds timestamp seconds
		if ((dots + i)->dot_timestamp_seconds != NULL)
		{
			sprintf(payload, "%s,\"timestamp\":%lu", payload, (dots + i)->dot_timestamp_seconds);
			// Adds timestamp milliseconds
			if ((dots + i)->dot_timestamp_millis != NULL)
			{
				char milliseconds[3];
				int timestamp_millis = (dots + i)->dot_timestamp_millis;
				uint8_t units = timestamp_millis % 10;
				uint8_t dec = (timestamp_millis / 10) % 10;
				uint8_t hund = (timestamp_millis / 100) % 10;
				sprintf(milliseconds, "%d%d%d", hund, dec, units);
				sprintf(payload, "%s%s", payload, milliseconds);
			}
			else
			{
				sprintf(payload, "%s000", payload);
			}
		}

		// Adds dot context
		if ((dots + i)->dot_context != NULL)
		{
			sprintf(payload, "%s,\"context\": {%s}", payload, (dots + i)->dot_context);
		}

		sprintf(payload, "%s}", payload);
		i++;

		if (i < currentValue)
		{
			sprintf(payload, "%s,", payload);
		}
		else
		{
			sprintf(payload, "%s}", payload);
			currentValue = 0;
		}
	}

	if (debug)
	{
		Serial.println("----------");
		Serial.println("payload:");
		Serial.println(payload);
		Serial.println("----------");
		Serial.println("");
	}
}

void UbiUtils::floatToChar(char *strValue, float value)
{
	char temp_arr[25];
	sprintf(temp_arr, "%17g", value);
	uint8_t j = 0;
	uint8_t k = 0;
	while (j < 25)
	{
		if (temp_arr[j] != ' ')
		{
			strValue[k] = temp_arr[j];
			k++;
		}
		if (temp_arr[j] == '\0')
		{
			strValue[k] = temp_arr[j];
			break;
		}
		j++;
	}
}

uint16_t UbiUtils::getPathLength(const char* deviceLabel, const char* variableLabel)
{
    return strlen("/api/v1.6/devices///lv") + strlen(deviceLabel) + strlen(variableLabel);	
}

uint16_t UbiUtils::getRequestLength(const char* path, const char* host, const char* token, const char* userAgent)
{
	return strlen("GET  HTTP/1.1\r\nHost: \r\nX-Auth-Token: "
                "\r\nUser-Agent: \r\nContent-Type: "
                "application/json\r\nConnection: close\r\n\r\n") +
                strlen(path) + strlen(host) + strlen(token) + strlen(userAgent);	
}

bool UbiUtils::syncronizeTime(const bool &debug)
{
	if(debug)
	{
        Serial.print(F("Setting time using SNTP"));
    }
    configTime(8 * 3600, 0, NTP_SERVER, NIST_SERVER);
    time_t now = time(nullptr);
    uint8_t attempts = 0;
    while (now < 8 * 3600 * 2 && attempts <= 5)
    {
        if (debug)
        {
            Serial.print(".");
        }
        now = time(nullptr);
        attempts += 1;
        delay(500);
    }

    if (attempts > 5)
    {
        if (debug)
        {
            Serial.println(F("[ERROR] Could not set time using remote SNTP to verify Cert"));
        }
        return false;
    }

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    if (debug)
    {
        Serial.print(F("Current time: "));
        Serial.print(asctime(&timeinfo));
    }
    return true;
}