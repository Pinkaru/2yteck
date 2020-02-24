/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
//#include "EEPROM.h"
#include "2yteck_eeprom.h"
#include "StateMachine.h"
//#include <WiFi.h>
#include "2yteck_wifi.h"
#include <WebSocketClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/* Global Variable */

char ssid[32]={0};
char pswd[32]={0};
/*
String wifi_string = "{\"ssid\" : \"ssid\", \"pswd\" : \"pswd\"}";
*/
char path[] = "/";
char host[] = "54.180.25.224";  //goorm ide nodejs test server
 
WebSocketClient webSocketClient;
WiFiClient client;

DynamicJsonDocument doc(1024);

struct JANGPAN_STATE st_state;


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");

        DeserializationError error = deserializeJson(doc, value);

        JsonObject obj = doc.as<JsonObject>();

        String strSsid = obj[String("ssid")];
        String strPswd = obj[String("pswd")];

        Serial.println(value.c_str());

        Serial.print("ssid: ");
        Serial.print(strSsid);
        Serial.print("\tpswd: ");
        Serial.print(strPswd);

        Serial.println();
        Serial.println("*********");

        strSsid.toCharArray(ssid,strSsid.length()+1);
        strPswd.toCharArray(pswd,strPswd.length()+1);

        Serial.print("ssid char array: ");
        Serial.println(ssid);

        Serial.print("pswd char array: ");
        Serial.println(pswd);

        ConnectWifi();

      }
    }
};

void setup() {
  Serial.begin(115200);
  //setup eeprom
  
  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to MyESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =)");

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.begin(115200);
  //wifi setup
}

void loop() {
  if(st_state.state == STOP){
    Serial.println("STOP State");
    st_state.state = SETTING;
  }
  else if(st_state.state == SETTING)
  {
    Serial.println("SETTING State");
    st_state.state = STOP;
  }
    

  delay(2000);
}
