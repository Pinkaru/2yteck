#include "2yteck_ble.h"
#include <Arduino.h>
void MyBLE_2YTECK::SetupBLE()
{
    // Create the BLE Device
    BLEDevice::init("UART Service");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    // Create the BLE Service
    pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                         CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_WRITE
                      );

    pRxCharacteristic->setCallbacks(this);

}

void MyBLE_2YTECK::BLEReceiveValue(std::string rxValue)
{
    Serial.print("Receive Value: ");
    Serial.println(rxValue.c_str());
}
void MyBLE_2YTECK::StartBLE()
{
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
            pTxCharacteristic->setValue(&txValue, 1);
            pTxCharacteristic->notify();
            txValue++;
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
