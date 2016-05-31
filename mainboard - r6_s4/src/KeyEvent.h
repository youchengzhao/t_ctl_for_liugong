/**
  ******************************************************************************
  * @file    temperature.h 
  * @author  Danny Zhao 
  * @version V2.0.0
  * @date    05/29/2016
  * @brief   
  ******************************************************************************
  **/
#ifndef _KEYEVENT_H_
#define _KEYEVENT_H_


#define LIMT_EVENT_COOL_ON              BIT0 
#define LIMT_EVENT_HEAT_ON              BIT1 


enum EVENT_TPYE = {
    EVENT_KEY_SET   = 1,  
    EVENT_KEY_POWER,  
    EVENT_KEY_UP   ,  
    EVENT_KEY_DOWN ,  
    EVENT_KEY_OPEN ,  
    EVENT_KEY_CLOSE,  
    EVENT_TYPE_END,
};

unsigned char getLimtEventFlag(void);
void keyPinInit(void);
void keyScan(void);


#endif
