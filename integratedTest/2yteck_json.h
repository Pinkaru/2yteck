#ifndef __2YTECK_JSON__
#define __2YTECK_JSON__

#include <ArduinoJson.h>
#include "StateMachine.h"

typedef enum
{
    LINE_ERR = -1,
    TEMP_SENSOR_ERR = -2,
    PRESSURE_SENSOR_ERR = -3,
    FIRE_DETECTED = -4,
    NO_MOVEMENT = -5,
}Jangpan_error_t;

class MyJSON_2YTECK: public MySTATE_2YTECK
{
    public:

        DynamicJsonDocument *doc;
        MyJSON_2YTECK();
        bool checkEvent(String str, String& event, String& action);
        String makeJsonString(Jangpan_error_t err, MySTATE_2YTECK state);
        
};

#endif //__2YTECK_JSON__