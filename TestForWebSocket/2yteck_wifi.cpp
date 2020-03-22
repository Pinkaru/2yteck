#include "2yteck_wifi.h"
// #include "eeprom/2yteck_eeprom.h"

bool MyWiFi_2YTECK::isWiFiConnected()
{
    char wifi_status;
    ReadEEPROM(EEPROM_WIFI_STATUS_OFFSET, 1, &wifi_status);
    if(wifi_status == 1)
    {
        return true;
    } else return false;
}

bool MyWiFi_2YTECK::ConnectWifi(char* ssid, char* pswd)
{
    WiFi.begin(ssid, pswd);

    unsigned long startTime = millis();;
    unsigned long currentTime = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        currentTime = millis();
        if(currentTime - startTime > 30000)   //30초 동안 try
        {
        Serial.println("Connect Failed");
        EEPROM.write(EEPROM_WIFI_OFFSET, 0);
        EEPROM.commit();  
        return false;
        }
    }
       
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    char wifiok = 1;
    WriteEEPROM(EEPROM_SSID_OFFSET, EEPROM_SSID_SIZE, ssid);
    WriteEEPROM(EEPROM_PSWD_OFFSET, EEPROM_SSID_SIZE, pswd);
    WriteEEPROM(EEPROM_WIFI_STATUS_OFFSET, 1, &wifiok);   
}

bool MyWiFi_2YTECK::SetupWiFi(char* ssid, char* pswd)
{
  bool iswifiConnected;
  if(iswifiConnected=isWiFiConnected())
  {
    Serial.println("Wifi Connect Try...");
    ReadEEPROM(EEPROM_SSID_OFFSET, EEPROM_SSID_SIZE, ssid);
    Serial.println(ssid);
    ReadEEPROM(EEPROM_PSWD_OFFSET, EEPROM_SSID_SIZE, pswd);
    Serial.println(pswd);
    iswifiConnected=ConnectWifi(ssid, pswd);
  }
  else{
    Serial.println("Need Wifi Setting");
  }
  return iswifiConnected;
}