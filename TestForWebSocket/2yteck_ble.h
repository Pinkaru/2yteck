#ifndef __2YTECK_BLE__
#define __2YTECK_BLE__

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

// #define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // UART service UUID
#define BEACON_UUID            "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_IN "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_OUT "beb5483e-36e1-4688-b7f5-ea07361b26fa"

// BLEServer *pServer = NULL;
// BLECharacteristic * pTxCharacteristic;
// BLEService *pService;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// uint8_t txValue = 0;

// unsigned long bleStartTime;
// unsigned long bleCurrentTime;

// class InCharEventCallback: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic* pChar);
//   // {
//   //   Serial.print("inCharEventCallback - onWrite: ");
//   //   Serial.println(pChar->getValue().c_str());
//   // }
// };

// class OutCharEventCallback: public BLECharacteristicCallbacks {
//   void onRead(BLECharacteristic* pChar) {
//     Serial.println("OutChareventCallback - onRead: ");
//   }
// };



class MyBLE_2YTECK: public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  public:
    BLEServer *pServer;
    BLECharacteristic *pTxCharacteristic;
    BLEService *pService;

    // InCharEventCallback *pCharInCallback;
    // OutCharEventCallback *pCharOutCallback;


    bool deviceConnected;
    bool oldDeviceConnected;
    bool bleReceiveSomething;

    char ssid[128];
    char pswd[128];
    char bleAddrStr[24];
    std::string bleAddrStr2;


    unsigned long bleStartTime;
    unsigned long bleCurrentTime;

    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        bleReceiveSomething = true;
        std::string rxValue = pCharacteristic->getValue();    
        if (rxValue.length() > 0) {
            BLEReceiveValue(rxValue);
        }
    }

    void onRead(BLECharacteristic* pChar) {
      Serial.print("OutChareventCallback - onRead: ");
      Serial.println(pChar->getValue().c_str());
    }

    void SetupBLE();
    void BLEReceiveValue(std::string rxValue);
    void StartBLE();
    void StopBLE();
    bool WorkBLE();
    void notifySet(std::string str);
};




#endif //__2YTECK_BLE__
