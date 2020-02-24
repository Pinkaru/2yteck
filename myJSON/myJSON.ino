#include "2yteck_json.h"
#include "StateMachine.h"

MyJSON_2YTECK myJson;
MySTATE_2YTECK myState;
void setup()
{
    Serial.begin(115200);
}

void loop()
{
    String test= "{\"event\":\"PRESSURE SENSOR ERR\", \"action\":\"on\"}";

    myJson.checkEvent(test);
    
    String test2 = myJson.makeJsonString(TEMP_SENSOR_ERR, myState);

    Serial.println(test2);
    delay(2000);
}
