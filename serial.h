#include<Windows.h>
#include<string>

#define SERIAL_RATE_1200    1200
#define SERIAL_RATE_2400    2400
#define SERIAL_RATE_4800    4800
#define SERIAL_RATE_9600    9600
#define SERIAL_RATE_19200   19200
#define SERIAL_RATE_38400   38400
#define SERIAL_RATE_57600   57600
#define SERIAL_RATE_115200  115200

class Serial{
    unsigned int BandRate;
    unsigned int Bufsize;
    HANDLE serial_handle=INVALID_HANDLE_VALUE;


    public:
    Serial();
    Serial(const unsigned int BandRate);

    bool init(const std::string com);
    bool send(const char data);
    bool send(const char *data,const unsigned int size);
    bool send(const std::string str);

    unsigned long available();
    unsigned char read();

};