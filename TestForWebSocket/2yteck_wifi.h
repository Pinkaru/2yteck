#ifndef __2YTECK_WIFI__
#define __2YTECK_WIFI__

#include <WiFi.h>
#include "2yteck_eeprom.h"

#define EEPROM_WIFI_STATUS_OFFSET 0

class MyWiFi_2YTECK: public MyEEPROM_2YTECK
{
    public:
        bool isWiFiConnected();
        bool ConnectWifi(char* ssid, char* pswd);
        bool SetupWiFi(char* ssid, char* pswd);
};

#endif //__2YTECK_WIFI__