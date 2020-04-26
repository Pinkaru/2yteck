/*
    1. Booting
        - Try WiFi Connect
    1-1. WiFi Connected
        - Run
            - Check Emergency
                - LED or Alarm
            - Timer: Send Status to Server  -- thread(?)
    1-2. WiFi Disconnected
        - RUN
            - Check Emergency       --thread
                - LED or Alarm
        - Try Reconnect
    2. Pushed Button            -- thread
        - BLE Button
            - BLE ON    
                - Timer: Connected
                - Receive WiFi ssid, password
        - Temperature Controller
            - Control Temperature
                - Serial control    
*/
//#include <WebSocketsClient.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>

#include "2yteck_wifi.h"
#include "2yteck_eeprom.h"
#include "2yteck_ble.h"
#include "StateMachine.h"
#include "2yteck_json.h"
#include <Thread.h>
#include <ThreadController.h>
#include <SoftwareSerial.h>
/*      Global Variable              */

#define __DEBUG__ 0
MyEEPROM_2YTECK myEEPROM;
MyBLE_2YTECK myBLE;
MyWiFi_2YTECK myWiFi;
MySTATE_2YTECK myState;
MyJSON_2YTECK myJSON;
WebSocketClient webSocketClient;
WiFiClient client;

char wifiSSID[128] = {0};
char wifiPSWD[128] = {0};
char socketPATH[] = "/";
char socketHOST[] = "clopi.it";
int  socketPort = 3000;
bool isWiFiConnected = false;
DynamicJsonDocument doc(1024);

Thread* pushedButton = new Thread();
Thread* checkEmergency = new Thread();
Thread* websocketThread = new Thread();
Thread* checkWiFiConnected = new Thread();
Thread* tempControllerSerial = new Thread();
Thread* bleThread = new Thread();

ThreadController controll = ThreadController();

bool isMessageForSend = false;
bool isMessageForReceive = false;
bool isCalledBleButton = false; // true test

const int ledWiFiGreen  =   18;
const int ledWiFiRed    =   19;
const int ledFireGreen  =   26;
const int ledFireRed    =   25;

/*
*   TempController status
    temperature 
    powerstate
    Outing
    errorstate
*/
unsigned int temperature = 15; //default value
bool powerState = false;
bool outing = false;
unsigned int errState = 0; // error nothing
int smokeDetectorValue =0;
bool smokeDetected = false;
void printStatus()
{
    Serial.println("*****\t Temperature State\t *****");
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Power State: ");
    if(powerState)
        Serial.println("ON");
    else 
        Serial.println("OFF");
    Serial.print("Outing State: ");
    if(outing)
        Serial.println("ON");
    else 
        Serial.println("OFF");
    Serial.print("Smoke Detection Value: ");
    Serial.println(smokeDetectorValue);
    // if(smokeDetected)
    // {
    //     Serial.println("Smoke Detected");
    //     digitalWrite(ledWiFiRed, 1);
    //     digitalWrite(ledWiFiGreen, 0);
    // }
}


/* 
 * Button Global Variable
*/

const int bleButtonPin = 16; //16
const int fireButtonPin = 17; //16
bool bleButtonONState = false;
bool bleBUttonPrevState = false;
bool bleButtonCurrState = false;
bool bleButtonWorked = false;   //button state
bool fireButtonONState = false;
bool fireBUttonPrevState = false;
bool fireButtonCurrState = false;
void buttonSetup()
{
    pinMode(bleButtonPin, INPUT_PULLUP);
    pinMode(fireButtonPin, INPUT_PULLUP);
}
void pushedButtonCallback() //50 millis   //ㅑf pull-up switch, need if state change
{
    //Serial.print("pushedButtonCallback\n");
    //Button Check
    //read button pin state
    //if set, save prevButtonState
    //after 30 milli seconds, if pin state hihg, set button state. -> change 30 -> 50
    //if prevButtonState is set, if read pin state is high, not work

    bleButtonCurrState = !(digitalRead(bleButtonPin));
    if(bleButtonCurrState && !isCalledBleButton)
    {
        if(!bleBUttonPrevState)
        {
            bleBUttonPrevState = true;
        }
        else
        {
            if(!bleButtonONState){
                Serial.print("BLE Button Press\n");
                bleButtonONState = true;
                isCalledBleButton = true;
            }
        }
    }
    else    //bleButton reset, button enable work
    {
        bleButtonONState = false;
        bleBUttonPrevState = false;
    }
    fireButtonCurrState = !(digitalRead(fireButtonPin));
    if(fireButtonCurrState)
    {
        if(!fireBUttonPrevState)
        {
            fireBUttonPrevState = true;
        }
        else
        {
            if(!fireButtonONState){
                Serial.print("Fire Button Press\n");
                fireButtonONState = true;
                digitalWrite(ledFireGreen, 1);
                digitalWrite(ledFireRed, 0);
            }
        }
    }
    else    //bleButton reset, button enable work
    {
        fireButtonONState = false;
        fireBUttonPrevState = false;
    }

}

/*
*   IR Smoke Detector Global Variable
*/
const int smokeDetectorReceivePin = 34;
const int smokeDetectorTransmitPin = 4;
// int smokeDetectorValue; //위로 이동
// 이동평균필터를 적용하기 위해서는 n개의 array필요. 메모리가 가능한지 검토후에 적용
// int smokeDetectorPrevValue;
// int smokeDetectorValueCount;
bool smokeDetectorWorkState = false;
bool smokeDetectorWorkStable = false;
unsigned long smokeDetectorWorkingStart;
unsigned long smokeDetectorWorkingEnd;
unsigned long smokeDetectorCheckDelayStart;
// bool smokeDetected = false; //위로 올림
int smokeEmergencyValue = 30;  //test
void smokeDetectorSetup()
{
    pinMode(smokeDetectorTransmitPin, OUTPUT);
    pinMode(smokeDetectorTransmitPin, 1);
    smokeDetectorWorkingStart = millis();
    smokeDetectorWorkState = true;
    // smokeDetectorValueCount =0;
}

void checkEmergencyCallback()
{
    //Serial.print("CheckEmergencyCallback\n");
    //check emergency
    //check ir gas checker
    //check human don't moving
    //led
    //alarm
    unsigned long smokeDetectorCurrTime = millis();
    if(smokeDetectorWorkState)
    {
        //감지기에 전원이 들어간 이후로 5초 대기(안정화)
        if((smokeDetectorCurrTime - smokeDetectorWorkingStart) > 5*1000)
        {
            if(!smokeDetectorWorkStable)
            {
                smokeDetectorWorkStable = true;
                smokeDetectorCheckDelayStart = millis();
            }
        }
        else{
            smokeDetectorWorkStable = false;
        }

        if(smokeDetectorWorkStable)
        {
            //woking stable, check value stable
            if((smokeDetectorCurrTime - smokeDetectorCheckDelayStart) > 10*1000)
            {
                smokeDetectorValue = analogRead(smokeDetectorReceivePin);
                #if __DEBUG__
                Serial.print("debugging... Smoke Detector value: ");
                Serial.println(smokeDetectorValue);
                #endif
            }
        }

        if(smokeDetectorValue > smokeEmergencyValue)
        {
            //alarm
            //led
            smokeDetected = true;
            Serial.println("Smoke Detected");
            digitalWrite(ledFireRed, 1);
            digitalWrite(ledFireGreen, 0);
            //when smoke detected reset?
        }
        else
        {
            smokeDetected = false;
        }
    }
}

/*
*   websocket client Global Variable
*/
bool isWebSocketConnected = false;
bool isClientConnected = false;
void websocketThreadCallback()
{
    //Serial.print("WebSocketThreadCallback\n");
    //check wifi connected -> wifi connected status bool check -> yes next
    if(isWiFiConnected)
    {
        if(!isClientConnected)
            if(client.connect(socketHOST, socketPort)){
                isClientConnected = true;
                Serial.print("WebSocket Client Connected\n");
            }
            else
                isClientConnected = false;
        if(!isWebSocketConnected)
        {
            webSocketClient.path = socketPATH;
            webSocketClient.host = socketHOST;
            if(webSocketClient.handshake(client))
                isWebSocketConnected = true;
            else
                isWebSocketConnected = false;
        }
        String webSocketgetData;
        if(client.connected())
        {
            webSocketClient.getData(webSocketgetData);
            if(webSocketgetData.length() > 0)
            {
                //websocket data check -> json string object parse
                //send Serial message -> callback
                String event;
                String action;
                myJSON.checkEvent(webSocketgetData, event, action);
                if(!strcmp(event.c_str(), "pw"))
                    if(!strcmp(action.c_str(),"on"))
                    {
                        Serial.print("power on\n");
                        powerState = true;
                        printStatus();
                    }
                        //serial send power on
                    else if(!strcmp(action.c_str(),"off")){
                        //serial send power off
                        Serial.print("power off\n");
                        powerState = false;
                        printStatus();
                    }
                if(!strcmp(event.c_str(), "temp"))
                    if(!strcmp(action.c_str(),"up"))
                    {
                        //serial send temp up
                        Serial.print("temperature up\n");
                        temperature++;
                        printStatus();
                    }
                    else if(!strcmp(action.c_str(),"down"))
                    {
                        //serial send temp down
                        Serial.print("temperature down\n");
                        temperature--;
                        printStatus();
                    }
                if(!strcmp(event.c_str(),"outing"))
                    if(!strcmp(action.c_str(),"on"))
                    {
                        //serial outing on
                        // Serial.print("outing on\n");
                        outing = true;
                        printStatus();
                    }
                    else if(!strcmp(action.c_str(),"off"))
                    {
                        //serial outing off
                        // Serial.print("outing off\n");
                        outing = false;
                        printStatus();
                    }
                if(!strcmp(event.c_str(),"status"))
                    //webscoekt send status
                    // Serial.print("status send\n");
                    printStatus();
            }
            //Check for messages to send -> bool pushed button or emergency
            //receive message
            //Check for messages to receive
            if(isMessageForSend)
            {
                //make String
                String makeJsonString;
                // char jsonSendMsg[128];
                // makeJsonString.toCharArray(jsonSendMsg, jsonSendMsg.length());
                webSocketClient.sendData(makeJsonString.c_str());
                //status data, send message
            }
        }
    }
}
/*
*   wifi connect thread
*/
void checkWiFiConnectedCallback()   // 1 minute period
{
    // Serial.print("CheckWiFiConnectedCallback\n");
    //check wifi connected -> wifi connected status bool check -> no next
        //try wifi connect
        //reconnected -> isWifiConnected = true
        //connection failed -> isWificonnected = false;   
    char ssid[128];
    char pswd[128];
    // char wifiConnectedStatusMsg[128];
    if(WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(ledWiFiGreen, 0);
        digitalWrite(ledWiFiRed, 1);
        isWiFiConnected = false;
        if(myWiFi.isWiFiConnected())
        {
            myEEPROM.ReadEEPROM(EEPROM_SSID_OFFSET, EEPROM_SSID_SIZE, ssid);
            myEEPROM.ReadEEPROM(EEPROM_PSWD_OFFSET, EEPROM_SSID_SIZE, pswd);
            WiFi.begin(ssid, pswd);
            // if(myBLE.deviceConnected)
            // {
            //     sprintf(wifiConnectedStatusMsg, "{\"status\":\"S\", \"mac\":\"%s\"}");
            //     myBLE.notifySet(bleAddrStr);
            // }
        }
    }
    else
    {
        isWiFiConnected = true;
        digitalWrite(ledWiFiGreen, 1);
        digitalWrite(ledWiFiRed, 0);
        // if(myBLE.deviceConnected)
        // {
        //     sprintf(wifiConnectedStatusMsg, "{\"status\":\"S\", \"mac\":\"%s\"}");
        //     myBLE.notifySet(bleAddrStr);
        // }
    }
}

/*
*   Serial Contorller Global Variable
*   Software Serail
*/
const int swSerialTx = NULL;
const int swSerialRx = NULL;
SoftwareSerial tempController;
void tempControllerSerialSetup()
{
    tempController.begin(115200, SWSERIAL_5N1, swSerialRx, swSerialTx, false);
}
void tempControllerSerialCallback()
{
    //Serial.print("TempControllerSerialCallback\n");
    //check if a controller message has been received
    static int testbool = 0;
    if(tempController.available() > 0)
    {
        //read, parse
    }
    if(isMessageForReceive)
    {
        //button callback or function
    }
    
    if(Serial.available() > 0)
    {
        String inputData = Serial.readString();
        Serial.print("serial Receive: ");
        Serial.print(inputData.c_str());
        // if(test)
        if(isCalledBleButton)
        // if(testbool)
        {
            Serial.println("notify test");
            myBLE.notifySet(inputData.c_str());
        }
        else
        {
            Serial.print("ble not set, testbool val: ");
            Serial.println(testbool);
        }

        if(isClientConnected)
        {
            DeserializationError error = deserializeJson(doc, inputData);
            JsonObject obj = doc.as<JsonObject>();
            String event = obj[String("event")];
            String action = obj[String("action")];
            char makeMsg[128];
            sprintf(makeMsg, "{\"event\" : \"%s\", \"action\" : \"%s\"}", event.c_str(), action.c_str());
            Serial.print(makeMsg);
            Serial.print("\n");

            webSocketClient.sendData(makeMsg);
        }

        //debugging
        if(!(strcmp(inputData.c_str(), "ble test\n")))
        {
            // testbool = 1;
            Serial.println("BLE Test start");
            isCalledBleButton = true;
            // myBLE.StartBLE();
        }

    

        
    }
}

/*
*   BLE Thread Callback Global Variable
*/
unsigned long bleThreadStart = NULL;
bool bleWorkingState = false;
void bleThreadSetup()
{
    myBLE.SetupBLE();
}
void bleThreadCallback()
{
    //Serial.print("BLEThreadCallback\n");
    //button event -> bool
    char wifiConnectedStatusMsg[128];
    unsigned long bleThreadCurrTime = millis();
    if(isCalledBleButton)
    {
        if(bleThreadStart == NULL)
            bleThreadStart = millis();
        if(!bleWorkingState)
        {
            bleWorkingState = true;
            myBLE.StartBLE();
        }
        if((bleThreadCurrTime - bleThreadStart) > 300 * 1000)
        {
            //ble stop
            myBLE.StopBLE();
            isCalledBleButton = false;
        }

        // Serial.print("debugging.... ssid: ");
        // Serial.print(myBLE.ssid);
        // Serial.print(" pswd:");
        // Serial.println(myBLE.pswd);
        // Serial.print("debugging... if(strmcp): ");
        // Serial.print(strcmp(myBLE.ssid,"NULL"));
        // Serial.print(", if(strmcp): ");
        // Serial.println(strcmp(myBLE.pswd,"NULL"));

        if((strcmp(myBLE.ssid,"NULL")) && (strcmp(myBLE.pswd,"NULL")))
        {
            // if(myBLE.bleReceiveSomething)
            // {
            //     Serial.println("bleReceiveSomething");
            //     WiFi.disconnect();
            //     myBLE.bleReceiveSomething = false;
            // }
            // Serial.println("debugging.... myBLE jump in if");
            // if(WiFi.status() != WL_CONNECTED)
            if(myBLE.bleReceiveSomething)
            {
                myBLE.bleReceiveSomething = false;
                Serial.print("ssid: ");
                Serial.print(myBLE.ssid);
                Serial.print(" pswd: ");
                Serial.println(myBLE.pswd);
                if(myWiFi.ConnectWifi(myBLE.ssid, myBLE.pswd))
                {
                    sprintf(wifiConnectedStatusMsg, "{\"status\":\"S\", \"mac\":\"%s\"}", myBLE.bleAddrStr);
                    myBLE.notifySet(wifiConnectedStatusMsg);
                }else
                {
                    sprintf(wifiConnectedStatusMsg, "{\"status\":\"F\", \"mac\":\"%s\"}", myBLE.bleAddrStr);
                    myBLE.notifySet(wifiConnectedStatusMsg);
                }
            }
            else
            {
                // myBLE.StopBLE();
            }
        }
        //ready for phone connection
        // isCalledBleButton = false;
    }
}

void setup()
{
    Serial.begin(115200);
    myEEPROM.SetupEEPROM();

    buttonSetup();
    pushedButton->onRun(pushedButtonCallback);
    pushedButton->setInterval(50);

    smokeDetectorSetup();
    checkEmergency->onRun(checkEmergencyCallback);
    checkEmergency->setInterval(1*1000);

    websocketThread->onRun(websocketThreadCallback);
    websocketThread->setInterval(2*1000);

    checkWiFiConnected->onRun(checkWiFiConnectedCallback);
    checkWiFiConnected->setInterval(5*1000);

    tempControllerSerialSetup();
    tempControllerSerial->onRun(tempControllerSerialCallback);
    tempControllerSerial->setInterval(200);

    bleThreadSetup();
    bleThread->onRun(bleThreadCallback);
    bleThread->setInterval(5*1000);

    isWiFiConnected=myWiFi.SetupWiFi(wifiSSID, wifiPSWD);

    controll.add(pushedButton);
    controll.add(checkEmergency);
    controll.add(websocketThread);
    controll.add(checkWiFiConnected);
    controll.add(tempControllerSerial);
    controll.add(bleThread);
 
    pinMode(ledFireRed, OUTPUT);
    pinMode(ledFireGreen, OUTPUT);
    pinMode(ledWiFiGreen, OUTPUT);
    pinMode(ledWiFiRed, OUTPUT);

    digitalWrite(ledFireRed, 0);
    digitalWrite(ledFireGreen, 1);
}
void loop()
{
    
    controll.run();

}
