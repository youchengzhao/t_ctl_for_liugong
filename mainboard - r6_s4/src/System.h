/**
  ******************************************************************************
  * @file    temperature.h 
  * @author  Danny Zhao 
  * @version V2.0.0
  * @date    05/29/2016
  * @brief   
  ******************************************************************************
  **/
#ifndef _System_H_
#define _System_H_


enum SYSTEM_STAT_TYPT = {
    SYSTEM_AUTO_CTL = 1,
    SYSTEM_SETTING,
    SYSTEM_COOLING,
    SYSTEM_HEATING,
    SYSTEM_HANDLE,
    SYSTEM_WARNING,
    SYSTEM_POWEROFF,
    SYSTEM_STAT_END,
};
enum SYSTEM_ERROR_TYPT = {
    SYSTEM_ERROR_MANY_KEY_DOWN = 1,
    SYSTEM_ERROR,
    SYSTEM_ERROR_END,
};
#endif
