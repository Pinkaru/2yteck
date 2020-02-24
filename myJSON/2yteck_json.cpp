#include "2yteck_json.h"

MyJSON_2YTECK::MyJSON_2YTECK()
{
    doc = new DynamicJsonDocument(1024);
}

bool MyJSON_2YTECK::checkEvent(String str)
{
    deserializeJson(*doc, str);
    JsonObject obj = doc->as<JsonObject>();

    String event = obj[String("event")];
    String action = obj[String("action")];

    Serial.print("event: ");
    Serial.print(event);
    Serial.print(", action:");
    Serial.println(action);

    return true;
}
String MyJSON_2YTECK::makeJsonString(Jangpan_error_t err, MySTATE_2YTECK state)
{
    String errName;
    String stateStr;
    String result;
    if(err == -1)
    {
        errName = "LINE ERR";
    }
    else if(err == -2)
    {
        errName = "TEMP SENSOR ERR";
    }
    else if(err == -3)
    {
        errName = "PRESSURE SENSOR ERR";
    }
    else if(err == -4)
    {
        errName = "FIRE DETECTED";
    }
    else if(err == -5)
    {
        errName = "NO MOVEMENT";
    }
    else
    {
        errName = "NOTHING";
    }

    result = "{\"event\":"+errName+", \"state\":\"test\"}";
    return result;
}