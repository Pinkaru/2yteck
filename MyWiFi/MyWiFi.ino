#include "2yteck_wifi.h"

MyWiFi_2YTECK myWiFi;
void setup()
{
    Serial.begin(115200);
    myWiFi = MyWiFi_2YTECK();
}

void loop()
{
    Serial.println("test");
    if(myWiFi.isWiFiConnected())
    {
        Serial.println("connected");
    }else
    {
        Serial.println("not connected");

    }
    delay(5000);
}