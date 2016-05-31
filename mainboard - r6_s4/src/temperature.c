/**
  ******************************************************************************
  * @file    temperature.c 
  * @author  Danny Zhao 
  * @version V2.0.0
  * @date    05/29/2016
  * @brief   
  ******************************************************************************
  **/

#include "config.h"
#include "temperature.h"
static unsigned int temperature_value_old_ = 0;
static unsigned int temperature_value_new_ = 0;
static unsigned int speed_old_ = 0;
static unsigned int speed_new_ = 0;
static unsigned int temperature_value_ = 0;
static unsigned char increment_ = 0; //增量
static unsigned char temperature_flag;


unsigned char getIcrement(void)
{
    return increment_;
}

//----------------------------ds18b20 driver-----------------------
static char initIC18b20()
{
  char error;
  DQ_out;
  _DINT();
  DQ0;
  __delay_cycles(8*500);
  DQ1;
  __delay_cycles(55*8);
  DQ_in;
  __delay_cycles(8);
  if (DQ_val)
    error = 1;
  else
    error = 0;
  DQ_out;
  DQ1;
  _EINT();
  __delay_cycles(400*8);
  return error; 
}

static void writeIC18b20(char data)
{
  char i;
  _DINT();
  for (i = 0; i< 8; i++)
  {
    DQ0;
    __delay_cycles(6*8);
    if(data & 0x01)
      DQ1;
    else
      DQ0;
    data >>=1;
    __delay_cycles(50*8);
    DQ1;
    __delay_cycles(10*8);
  }
  _EINT();
}
static char readIC18b20()
{
  char i;
  char tmp;
  _DINT();
  for(i = 0; i < 8; i++)
  {
    tmp >>=1;
    DQ0;
    __delay_cycles(6*8);
    DQ1;
    __delay_cycles(8*8);
    DQ_in;
    _NOP();
    if (DQ_val)
        tmp |= 0x80;
    __delay_cycles(45*8);
    DQ_out;
    __delay_cycles(10*8);
  }
  _EINT();
  return tmp;
}
static void skip()
{
    write_18b20(0xCC);
}
static void convert()
{
  write_18b20(0x44);
}
static void readSP()
{
  write_18b20(0xbe);
}

static unsigned int readTmp()
{
  char low;
  unsigned int temp;
  low = read_18b20();
  temp = read_18b20();
  temp = (temp<<8) | low;
  return temp;
}


/**
  * @brief  get temperature function .
  * @param  None
  * @retval : 1->return temperature value  the next time  called
  */
unsigned int doConvert()
{ 
  static char t_flag = 0;
  char i;
  char low;
  unsigned int temp, value;
  
  if (t_flag == 0) 
  {
        
     for (i = 0; i < 2; i++)
     {
       temp = init_18b20();
       if (!temp)
         break;
     }
     if (i == 2)
     {
       t18b20_flag = 1;
       return 0;
     }
     else
     {
         t18b20_flag = 0;
     }
     
     write_18b20(0xcc);
     write_18b20(0x44);
     t_flag = 1;
     return 1;
  }
  else
  {
     t_flag = 0;
     for (i = 0; i < 2; i++)
     {
       temp = init_18b20();
       if (!temp)
         break;
     }
     if (i == 2)
     {
       t18b20_flag = 1;
       return 0;
     }
     else
     {
         t18b20_flag = 0;
     }
     write_18b20(0xcc);
     write_18b20(0xbe);
     low = read_18b20();
     temp = read_18b20();
     temp = (temp << 8) | low;
     if (temp < 0xfff)
       t18b20_flag = 0;
     else
       t18b20_flag = 2;
     
     value = temp * (0.625);
     temperature_value_ = value;
     return value;
    }
}

void calculatSpeed(void)
{
   if(temperature_value_old_ == 0;) 
       temperature_value_old_ = temperature_value_;
   if(temperature_value_new_ == 0;) 
       temperature_value_new_ = temperature_value_;

   speed_old_ = speed_new_;
   if (temperature_value_new_ > temperature_value_old_)
   {
       speed_new_ = temperature_value_new_ - temperature_value_old_;
       if (speed_new_ > 0 )
       {
          temperature_flag = TEMPERATURE_FLAG_UP;  
       }
   }
   else
   {
       speed_new_ =  temperature_value_old_ - temperature_value_new_;
       if (speed_new_ > 0 )
       {
          temperature_flag = TEMPERATURE_FLAG_DOWN;  
       }
   }
   if(speed_new_ >= TEMPERATURE_MIN_SPACE )
   {
       if (speed_new_ < TEMPERATURE_MAX_SPACE)
       {
           increment_ = speed_new_* INCREMENT_BASE;
       }
       else
       {
           increment_ =  INCREMENT_MAX;
       }
   }
   else
   {
      increment_ = 0; 
   }
}


