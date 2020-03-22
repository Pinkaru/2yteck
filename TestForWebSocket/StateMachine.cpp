#include "StateMachine.h"
#include <Arduino.h>
MySTATE_2YTECK::MySTATE_2YTECK()
{
    state = STOP;
}

stateOfjangpan MySTATE_2YTECK::currentState()
{
    return state;
}
stateOfjangpan MySTATE_2YTECK::changeState(stateOfjangpan value)
{
    state = value;
    return state;
}

void MySTATE_2YTECK::toString()
{
    // stateOfjangpan currentState = currentState();

    if(currentState() == STOP)
        Serial.println("Current Stata is STOP");
    else if(currentState() == SETTING)
        Serial.println("Current Stata is SETTING");
    else if(currentState() == RECEIVER)
        Serial.println("Current Stata is RECEIVER");
    else if(currentState() == TRANSMITTER)
        Serial.println("Current Stata is TRANSMITTER");
    else if(currentState() == ALERT)
        Serial.println("Current Stata is ALERT");
}