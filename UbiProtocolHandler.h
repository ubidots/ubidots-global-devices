#ifndef __UBIPROTOCOLHANDLER_H__
#define __UBIPROTOCOLHANDLER_H__

#include "UbiTypes.h"
#include "UbiUtils.h"

class UbiProtocolHandler
{
    public:
        explicit UbiProtocolHandler(const IotProtocol& protocol) { _protocol = protocol; };
        virtual ~UbiProtocolHandler() { delete _dots; };

        bool connect();
        bool connected();
        bool reconnect();
        bool serverConnected();
		void setDebug(const bool& debug);
        void addContext(char* key, char* value);
        void init();
        virtual void getContext(char* context_result) = 0;
        virtual bool send(const char* deviceLabel, const char* deviceName) = 0;
        virtual void add(const char *variable_label, double value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis) = 0;
        virtual double get(const char *device_label, const char *variable_label) = 0;



    protected:

        bool _debug{false};
        IotProtocol _protocol;
        bool _dirty{false};
        Value* _dots{(Value *)malloc(MAX_VALUES * sizeof(Value))};
        int8_t _currentValue{0};
        int _connectionTimeout = 5000;
        ContextUbi* _context;
        int8_t _currentContext = 0;

    private:

};


template<IotProtocol Protocol>
class ProtocolHandler : public UbiProtocolHandler
{
    public:
        explicit ProtocolHandler(const IotProtocol& protocol) : UbiProtocolHandler(protocol) { }
        virtual ~ProtocolHandler() {  }

        bool send(const char* deviceLabel, const char* deviceName);
        void add(const char *variable_label, double value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis);
        double get(const char *device_label, const char *variable_label);
        void getContext(char* context_result);

    protected:

    private:

};







#endif // __UBIPROTOCOLHANDLER_H__