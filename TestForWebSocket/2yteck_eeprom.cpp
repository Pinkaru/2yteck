#include "2yteck_eeprom.h"

// MyEEPROM_2YTECK::MyEEPROM_2YTECK()
// {
//     SetupEEPROM();
// }

bool MyEEPROM_2YTECK::WriteEEPROM(int start, int size, char * data)
{
    if(start >= EEPROM_SIZE) return false;
    for(int romIndex = start, dataIndex =0; romIndex<start + size; romIndex++, dataIndex++)
    {
        EEPROM.write(romIndex,data[dataIndex]);
    }
    EEPROM.commit();
    return true;
}
bool MyEEPROM_2YTECK::ReadEEPROM(int start, int size, char * data)
{
    if(start >= EEPROM_SIZE) return false;
    for(int romIndex = start, dataIndex = 0; romIndex<start+size; romIndex++, dataIndex++)
    {
        data[dataIndex] = EEPROM.read(romIndex);
    }
    return true;
}

void MyEEPROM_2YTECK::SetupEEPROM()
{
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("failed to initialise EEPROM"); delay(1000000);
    }
}