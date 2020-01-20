#ifndef __2YTECK_WIFI__
#define __2YTECK_WIFI__

#include <WiFi.h>
#define EEPROM_WIFI_STATUS_OFFSET 0
bool isWiFiConnected()
{
    char wifi_status = EEPROM.read(EEPROM_WIFI_STATUS_OFFSET);
    if(wifi_status == 1)
    {
        return true;
    } else return false;
}

#endif //__2YTECK_WIFI__