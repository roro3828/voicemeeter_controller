#include<Windows.h>
#include<string>
#include<iostream>

#define MAX 64

class Serial{
    unsigned int BandRate;
    unsigned int Bufsize;
    HANDLE serial_handle=INVALID_HANDLE_VALUE;
    public:
    Serial(){
        this->BandRate=9600;
        this->Bufsize=1024;
    }

    bool init(std::string com){
        bool ret;
        if(this->serial_handle!=INVALID_HANDLE_VALUE){
            CloseHandle(this->serial_handle);
        }

        this->serial_handle=CreateFileA(com.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

        if(this->serial_handle==INVALID_HANDLE_VALUE){
            std::cout<<"PORT ERROR"<<std::endl;
            return false;
        }

        ret=SetupComm(this->serial_handle,this->Bufsize,this->Bufsize);
        if(!ret){
            std::cout<<"setup failed"<<std::endl;
            return false;
        }

        ret=PurgeComm(this->serial_handle,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
        if(!ret){
            std::cout<<"clear failed"<<std::endl;
            return false;
        }

        DCB dcb;
        GetCommState(this->serial_handle,&dcb);
        dcb.DCBlength=sizeof(DCB);
        dcb.BaudRate=this->BandRate;
        dcb.fBinary=true;
        dcb.ByteSize=8;
        dcb.fParity=NOPARITY;
        dcb.StopBits=ONESTOPBIT;
        dcb.fOutxCtsFlow=false;
        dcb.fOutxDsrFlow=false;
        dcb.fDtrControl=DTR_CONTROL_DISABLE;
        dcb.fRtsControl=RTS_CONTROL_DISABLE;
        dcb.fOutX=false;
        dcb.fInX=false;
        dcb.fTXContinueOnXoff=true;
        dcb.XonLim=512;
        dcb.XoffLim=512;
        dcb.XonChar=0x11;
        dcb.XoffChar=0x13;
        dcb.fNull=true;
        dcb.fAbortOnError=true;
        dcb.fErrorChar=false;
        dcb.ErrorChar=0x00;
        dcb.EofChar=0x03;
        dcb.EvtChar=0x02;

        ret=SetCommState(this->serial_handle,&dcb);
        if(!ret){
            std::cout<<"setcommstate failed"<<std::endl;
            return false;
        }

        COMMTIMEOUTS timeout;
        timeout.ReadIntervalTimeout=500;
        timeout.ReadTotalTimeoutMultiplier=0;
        timeout.ReadTotalTimeoutConstant=500;
        timeout.WriteTotalTimeoutMultiplier=0;
        timeout.WriteTotalTimeoutConstant=500;
        ret=SetCommTimeouts(this->serial_handle,&timeout);
        if(!ret){
            std::cout<<"SetCommTimeouts failed"<<std::endl;
            return false;
        }

        return true;
    }
    bool send(char data){
        DWORD dwSendSize;
        COMSTAT Comstat;
        DWORD dwErrorMask;

        constexpr auto DataSize=sizeof(data);

        while(true){
            ClearCommError(this->serial_handle,&dwErrorMask,&Comstat);

            if((MAX-Comstat.cbOutQue)>DataSize&&(Comstat.fCtsHold==false)){
                break;
            }
            std::cout << "stack over";

            switch (dwErrorMask) {
            case CE_BREAK: {
                std::cout << "break";
                break;
            }
            case CE_RXPARITY: {
                std::cout << "parity";
                break;
            }
            case CE_RXOVER: {
                std::cout << "over";
                break;
            }default: {
                std::cout << "unknown";
                break;
            }
            }
            return false;
        }

        if(WriteFile(this->serial_handle,&data,sizeof(data),&dwSendSize,NULL)==0){
            std::cout<<"SEND FAILED"<<std::endl;
            return false;
        }
        return true;
    }

    bool send(char *data,unsigned int size){
        for(int i=0;i<size;i++){
            if(!this->send(data[i])){
                return false;
            }
        }
        return true;
    }
};