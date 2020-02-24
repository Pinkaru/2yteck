#ifndef __2YTECK_STATEMACHINE__
#define __2YTECK_STATEMACHINE__

typedef enum  
{
    STOP = 0,
    SETTING,
    RECEIVER,
    TRANSMITTER,
    ALERT
}stateOfjangpan;


class MySTATE_2YTECK
{
    public:
        MySTATE_2YTECK();
        stateOfjangpan state;
        stateOfjangpan currentState();
        stateOfjangpan changeState(stateOfjangpan value);
        stateOfjangpan calcState();
        void toString();
};
#endif //__2YTECK_STATEMACHINE__