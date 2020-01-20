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

//#define EEPROM_SIZE 512
//#define EEPROM_SSID_OFFSET 1
//#define EEPROM_SSID_SIZE 32
//#define EEPROM_PSWD_OFFSET 33
//#define EEPROM_WIFI_OFFSET 0

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

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
bool ConnectWifi()
{
   WiFi.begin(ssid, pswd);

   unsigned long startTime = millis();;
   unsigned long currentTime = 0;
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      currentTime = millis();
      if(currentTime - startTime > 30000)
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
  
//  for(int i=EEPROM_SSID_OFFSET, count =0; i<EEPROM_SSID_OFFSET+EEPROM_SSID_SIZE; i++, count++)
//    EEPROM.write(i, ssid[count]);
//    EEPROM.commit();  
//  for(int i=EEPROM_PSWD_OFFSET, count=0; i<EEPROM_PSWD_OFFSET+EEPROM_SSID_SIZE; i++, count++)
//    EEPROM.write(i, pswd[count]);
//    EEPROM.commit();  
//  EEPROM.write(EEPROM_WIFI_OFFSET, 1);
//  EEPROM.commit();  
   
}
//
//bool CheckWifi()
//{
//  char wifi_status = EEPROM.read(EEPROM_WIFI_OFFSET);
//  Serial.print("WiFi Status Check: ");
//  Serial.println(wifi_status);
//  if(wifi_status == 1)
//  {
//    Serial.println("Wifi already setting");
//    return true;
//  }
//  else
//    return false;
//}


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
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
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
//  if(CheckWifi())
  if(isWiFiConnected())
  {
    Serial.println("Wifi Connect Try...");
    for(int i=EEPROM_SSID_OFFSET, count =0; i<EEPROM_SSID_OFFSET+EEPROM_SSID_SIZE; i++, count++)
      ssid[count] = EEPROM.read(i);
      Serial.println(ssid);
    for(int i=EEPROM_PSWD_OFFSET, count =0; i<EEPROM_PSWD_OFFSET+EEPROM_SSID_SIZE; i++, count++)
      pswd[count] = EEPROM.read(i);
      Serial.println(pswd);
    ConnectWifi();
  }
  else{
    Serial.println("Need Wifi Setting");
  }
}

void loop() {
//
//    String data;
// 
//  if (client.connected()) {
//    Serial.print("client connected\t");
// 
//    webSocketClient.sendData("Test for BT+WiFi");
////    Serial.print(sendOpenMsg);
////    webSocketClient.sendData(sendReqMsg);
////    Serial.print(sendReqMsg);
// 
//    webSocketClient.getData(data);
//    if (data.length() > 0) {
//      Serial.print("Received data: ");
//      Serial.println(data);
//    }
// 
//  } else {
//    Serial.println("Client disconnected.");
//  }
//  // put your main code here, to run repeatedly:
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
