#include"serial.h"
#include<iostream>
#include<string>

int main(){
    Serial serial;
    serial.init("COM6");
    std::string str;
    int c=-2;
    unsigned long val=0;
    while(str!="end"){
        while(serial.available()==0){
            Sleep(1);
        }

        unsigned long tmp=serial.read();
        if(c==-2){
            if(tmp==0xFF){
                c=-1;
            }
        }
        else if(c==-1){
            if(tmp==0x01){
                c=0;
            }
            else{
                c=-2;
            }
        }
        else{
            val+=(tmp<<(c*8));
            c++;

            if(c==4){
                printf("val= %u\n",val);

                if(val<4096){
                    float data=((float)(val*72))/4095;
                    printf("Gain= %f\n",data-60.0f);
                }
                c=-2;
                val=0;
            }
        }
    }
}