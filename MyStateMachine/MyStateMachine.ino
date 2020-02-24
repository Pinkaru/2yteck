#include "StateMachine.h"

MySTATE_2YTECK myState;

void setup()
{
    Serial.begin(115200);
    // myState = new MySTATE_2YTECK();
}

void loop()
{

    myState.toString();
    if(myState.currentState() == STOP)
        myState.changeState(SETTING);
    else if(myState.currentState() == SETTING)
        myState.changeState(RECEIVER);
    else if(myState.currentState() == RECEIVER)
        myState.changeState(TRANSMITTER);
    else if(myState.currentState() == TRANSMITTER)
        myState.changeState(ALERT);
    else if(myState.currentState() == ALERT)
        myState.changeState(STOP);
    
    delay(1000);
}