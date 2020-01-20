#include "2yteck_eeprom.h"


bool WriteEEPROM(int start, int size, char * data)
{
    if(start >= EEPROM_SIZE) return false;
    for(int romIndex = start, dataIndex =0; romIndex<start + size; romIndex++, dataIndex++)
    {
        EEPROM.write(romIndex,data[dataIndex]);
    }
    EEPROM.commit();
    return true;
}
bool ReadEEEPROM(int start, int size, char * data)
{
    if(start >= EEPROM_SIZE) return false;
    for(int romIndex = start, dataIndex = 0; romIndex<start+size; romIndex++, dataIndex++)
    {
        data[dataIndex] = EEPROM.read(romIndex);
    }
    return true;
}