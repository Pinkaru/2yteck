#include "2yteck_ble.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_bt_device.h>
DynamicJsonDocument bleJsonDoc(512);
void MyBLE_2YTECK::SetupBLE()
{
    // Create the BLE Device
    BLEDevice::init("2YTECK_SmokeDetector");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    const uint8_t* bdAddr = esp_bt_dev_get_address();
    sprintf(bleAddrStr, "%02x:%02x:%02x:%02x:%02x:%02", bdAddr[0], bdAddr[1], bdAddr[2], bdAddr[3], bdAddr[4], bdAddr[5]);
    Serial.print("BLE Address: ");
    Serial.print(bleAddrStr);
    bleAddrStr2 += bleAddrStr;
    Serial.print(bleAddrStr2.c_str());
    // Create the BLE Service
    pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_OUT,
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_READ
                    );

    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                         CHARACTERISTIC_UUID_IN,
                        BLECharacteristic::PROPERTY_WRITE
                      );

    // pRxCharacteristic->setCallbacks(this);
    // pCharInCallback = new InCharEventCallback();
    // pCharOutCallback = new OutCharEventCallback();
    pTxCharacteristic->setCallbacks(this);
    pRxCharacteristic->setCallbacks(this);

}

void MyBLE_2YTECK::BLEReceiveValue(std::string rxValue)
{
    Serial.print("Receive Value: ");
    Serial.println(rxValue.c_str());
    
    DeserializationError error = deserializeJson(bleJsonDoc, rxValue);
    JsonObject obj = bleJsonDoc.as<JsonObject>();

    String strSsid = obj[String("ssid")];
    String strPswd = obj[String("pswd")];

    Serial.print("ssid: ");
    Serial.print(strSsid);
    Serial.print("\tpswd: ");
    Serial.print(strPswd);
    Serial.print("\n");

    strSsid.toCharArray(ssid,strSsid.length()+1);
    strPswd.toCharArray(pswd,strPswd.length()+1);
}
void MyBLE_2YTECK::StartBLE()
{
    
    sprintf(ssid, "NULL");
    sprintf(pswd, "NULL");
    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("Start BLE");
    bleStartTime = millis();
}
void MyBLE_2YTECK::StopBLE()
{
    pService->stop();
    pServer->getAdvertising()->stop();
    Serial.println("Stop BLE");

    sprintf(ssid, "NULL");
    sprintf(pswd, "NULL");
    

}
bool MyBLE_2YTECK::WorkBLE()
{
    bleCurrentTime = millis();
    if(bleCurrentTime - bleStartTime > 30000){
        Serial.println("Working time over");
        StopBLE();
        return false;
    }
        
    else 
    {
        if (deviceConnected) {
            
            pTxCharacteristic->setValue(bleAddrStr2);
            pTxCharacteristic->notify();
            delay(10); // bluetooth stack will go into congestion, if too many packets are sent
        }

        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            Serial.println("start advertising");
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }
    }
    return true;
}

void MyBLE_2YTECK::notifySet(std::string str)
{
    pTxCharacteristic->setValue((uint8_t*)str.c_str(), strlen(str.c_str()));
    pTxCharacteristic->notify();
}

