#ifndef __2YTECK_BLE__
#define __2YTECK_BLE__

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// BLEServer *pServer = NULL;
// BLECharacteristic * pTxCharacteristic;
// BLEService *pService;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// uint8_t txValue = 0;

// unsigned long bleStartTime;
// unsigned long bleCurrentTime;

class MyBLE_2YTECK: public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  public:
    BLEServer *pServer;
    BLECharacteristic *pTxCharacteristic;
    BLEService *pService;
    bool deviceConnected;
    bool oldDeviceConnected;

    uint8_t txValue = 0;

    unsigned long bleStartTime;
    unsigned long bleCurrentTime;

    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();    
        if (rxValue.length() > 0) {
            BLEReceiveValue(rxValue);
        }
    }

    void SetupBLE();
    void BLEReceiveValue(std::string rxValue);
    void StartBLE();
    void StopBLE();
    bool WorkBLE();
};




#endif //__2YTECK_BLE__
