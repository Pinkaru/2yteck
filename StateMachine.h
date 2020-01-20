#ifndef __2YTECK_STATEMACHINE__
#define __2YTECK_STATEMACHINE__

typedef enum  
{
    STOP = 0,
    SETTING,
    RECEIVER,
    TRANSMITTER,
    ALERT
}state_t;

struct JANGPAN_STATE
{
    state_t state;   
};
#endif //__2YTECK_STATEMACHINE__