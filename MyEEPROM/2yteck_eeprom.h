#ifndef __2YTECK_EEPROM__
#define __2YTECK_EEPROM__
#include "EEPROM.h"

#define EEPROM_SIZE 512
#define EEPROM_SSID_OFFSET 1
#define EEPROM_SSID_SIZE 32
#define EEPROM_PSWD_OFFSET 33
#define EEPROM_WIFI_OFFSET 0

class MyEEPROM_2YTECK
{
    public:
        // MyEEPROM_2YTECK();
        bool WriteEEPROM(int start, int size, char * data);
        bool ReadEEPROM(int start, int size, char * data);
        void SetupEEPROM();
};




#endif //__2YTECK_EEPROM__