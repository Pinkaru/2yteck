/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

// #include "2yteck_eeprom.h"
// #include "StateMachine.h"
// #include "2yteck_wifi.h"
#include "2yteck_ble.h"

#include <ArduinoJson.h>
#include <WebSocketClient.h>

/* Global Variable */

MyBLE_2YTECK myBLE;

void setup() {
  Serial.begin(115200);
  myBLE = MyBLE_2YTECK();
  myBLE.SetupBLE();
}

bool stateOfBLE = false;

void loop() {
  int numberOfReceive;
  char receiveChar[128] = {0};
  int i;
  if(stateOfBLE == false){
    if(numberOfReceive = Serial.available() > 0)
    {
      Serial.print("Receive Char Num: ");
      Serial.println(numberOfReceive);
      for(int i=0; i<numberOfReceive; i++)
        receiveChar[i] = Serial.read();
      Serial.println(receiveChar);
      if(receiveChar[0] == '0')
      {
        myBLE.StartBLE();
        stateOfBLE = true;
      }
    }  
  }
  else if(stateOfBLE == true)
  {
    stateOfBLE=myBLE.WorkBLE();
  }
}
