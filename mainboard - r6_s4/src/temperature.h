/**
  ******************************************************************************
  * @file    temperature.h 
  * @author  Danny Zhao 
  * @version V2.0.0
  * @date    05/29/2016
  * @brief   
  ******************************************************************************
  **/

#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#define TEMPERATURE_FLAG_UP        0X01
#define TEMPERATURE_FLAG_DOWN      0X02
#define TEMPERATURE_MIN_SPACE      1  //0.2 
#define TEMPERATURE_MAX_SPACE      6  //0.6
#define INCREMENT_BASE             20  //增量
#define INCREMENT_MAX              100  //增量

unsigned int doConvert(void);
void calculatSpeed(void);
unsigned char getIcrement(void);



#endif
