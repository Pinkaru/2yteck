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

MyEEPROM_2YTECK myEEPROM;
MyBLE_2YTECK myBLE;
MyWiFi_2YTECK myWiFi;
MySTATE_2YTECK myState;
MyJSON_2YTECK myJSON;
WebSocketClient webSocketClient;
WiFiClient client;

char wifiSSID[128] = {0};
char wifiPSWD[128] = {0};
char socketPATH[128] = "/";
char socketHOST[128] = "";
int  socketPort = 9999;
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
bool isCalledBleButton = false;

/* 
 * Button Global Variable
*/

const int bleButtonPin = NULL; //아직 미정
bool bleButtonONState = false;
bool bleBUttonPrevState = false;
bool bleButtonCurrState = false;
bool bleButtonWorked = false;   //button state
void buttonSetup()
{
    pinMode(bleButtonPin, INPUT);
}
void pushedButtonCallback() //50 millis   //ㅑf pull-up switch, need if state change
{
    //Button Check
    //read button pin state
    //if set, save prevButtonState
    //after 30 milli seconds, if pin state hihg, set button state. -> change 30 -> 50
    //if prevButtonState is set, if read pin state is high, not work

    bleButtonCurrState = digitalRead(bleButtonPin);
    if(bleButtonCurrState && !isCalledBleButton)
    {
        if(!bleBUttonPrevState)
        {
            bleBUttonPrevState = true;
        }
        else
        {
            if(!bleButtonONState){
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
}

/*
*   IR Smoke Detector Global Variable
*/
const int smokeDetectorReceivePin = 34;
const int smokeDetectorTransmitPin = 4;
int smokeDetectorValue;
// 이동평균필터를 적용하기 위해서는 n개의 array필요. 메모리가 가능한지 검토후에 적용
// int smokeDetectorPrevValue;
// int smokeDetectorValueCount;
bool smokeDetectorWorkState = false;
bool smokeDetectorWorkStable = false;
unsigned long smokeDetectorWorkingStart;
unsigned long smokeDetectorWorkingEnd;
unsigned long smokeDetectorCheckDelayStart;
bool smokeDetected = false;
int smokeEmergencyValue = 200;  //test
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
    //check emergency
    //check ir gas checker
    //check human don't moving
    //led
    //alarm
    unsigned long smokeDetectorCurrTime = millis();
    if(smokeDetectorWorkState)
    {
        //감지기에 전원이 들어간 이후로 20초 대기(안정화)
        if((smokeDetectorCurrTime - smokeDetectorWorkingStart) > 20*1000)
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
            if((smokeDetectorCurrTime - smokeDetectorCheckDelayStart) > 30*1000)
            {
                smokeDetectorValue = analogRead(smokeDetectorReceivePin);
            }
        }

        if(smokeDetectorValue > smokeEmergencyValue)
        {
            //alarm
            //led
            smokeDetected = true;
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
    //check wifi connected -> wifi connected status bool check -> yes next
    if(isWiFiConnected)
    {
        if(!isClientConnected)
            if(client.connect(socketHOST, socketPort))
                isClientConnected = true;
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
                if(event == "pw")
                    if(action == "on")
                        //serial send power on
                        Serial.print("nothing");
                    else if(action == "off")
                        //serial send power off
                        Serial.print("nothing");
                else if(event == "temp")
                    if(action == "up")
                        //serial send temp up
                        Serial.print("nothing");
                    else if(action == "down")
                        //serial send temp down
                        Serial.print("nothing");
                else if(event == "outing")
                    if(action == "on")
                        //serial outing on
                        Serial.print("nothing");
                    else if(action == "off")
                        //serial outing off
                        Serial.print("nothing");
                else if(event == "status")
                    //webscoekt send status
                    Serial.print("nothing");
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
    //check wifi connected -> wifi connected status bool check -> no next
        //try wifi connect
        //reconnected -> isWifiConnected = true
        //connection failed -> isWificonnected = false;   
    char ssid[128];
    char pswd[128];
    if(WiFi.status() != WL_CONNECTED)
    {
        isWiFiConnected = false;
        if(myWiFi.isWiFiConnected())
        {
            myEEPROM.ReadEEPROM(EEPROM_SSID_OFFSET, EEPROM_SSID_SIZE, ssid);
            myEEPROM.ReadEEPROM(EEPROM_PSWD_OFFSET, EEPROM_SSID_SIZE, pswd);
            WiFi.begin(ssid, pswd);
        }
    }
    else
    {
        isWiFiConnected = true;
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
    tempController.begin(115200);
}
void tempControllerSerialCallback()
{
    //check if a controller message has been received
    if(tempController.available() > 0)
    {
        //read, parse
    }
    if(isMessageForReceive)
    {
        //button callback or function
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
    //button event -> bool
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

        //ready for phone connection
        isCalledBleButton = false;
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
    checkEmergency->setInterval(5*1000);

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
}
/*
*   1. Booting
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
void loop()
{
    controll.run();
}