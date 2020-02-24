#include "2yteck_eeprom.h"

MyEEPROM_2YTECK myEEPROM;

void setup()
{
    Serial.begin(115200);
    myEEPROM = MyEEPROM_2YTECK();
    myEEPROM.SetupEEPROM();

}

void loop()
{
    Serial.println("test........");
    int recvNum;
    char receiveData[128];

    int i;

    char saveNum;
    if(recvNum = Serial.available())
    {
        for(i=0; i<recvNum; i++)
            receiveData[i] = Serial.read();
        Serial.print("receiveData: ");
        Serial.println((char *)receiveData);
        saveNum = recvNum;
        myEEPROM.WriteEEPROM(0,1,&saveNum);
        for(i=0; i<recvNum; i++)
            myEEPROM.WriteEEPROM(i+1, 1, &receiveData[i]);
    }

    char checkNum;
    char readData[128];
    myEEPROM.ReadEEPROM(0,1,&checkNum);
    if(checkNum > 0)
    {
        myEEPROM.ReadEEPROM(1,checkNum,readData);
        Serial.print("EEPROM data: ");
        Serial.println((char *)readData);
    }

    delay(2000);
}