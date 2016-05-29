#include "msp430g2332.h"
#include "config.h"
#include "queue.h"


static unsigned char system_stat_ ;    
static unsigned int  system_time_ ; 
static unsigned char system_time_flag_ ; 
static unsigned char system_error_flag_ ; 
static unsigned char limt_event_flag_; 
static struct queue_type key_queue;
static char chip_id_check[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                               0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,};

#define SET_TIME_1MS_FLAG() system_time_flag_ |= 0X01
#define SET_TIME_10MS_FLAG() system_time_flag_ |= 0X02
#define SET_TIME_100MS_FLAG() system_time_flag_ |= 0X04
#define SET_TIME_1000MS_FLAG() system_time_flag_ |= 0X08

#define GET_TIME_1MS_FLAG() system_time_flag_ &= 0X01
#define GET_TIME_10MS_FLAG() system_time_flag_ &= 0X02
#define GET_TIME_100MS_FLAG() system_time_flag_ &= 0X04
#define GET_TIME_1000MS_FLAG() system_time_flag_ &= 0X08

#define CLEAN_TIME_1MS_FLAG() system_time_flag_ &= ~0X01
#define CLEAN_TIME_10MS_FLAG() system_time_flag_ &= ~0X02
#define CLEAN_TIME_100MS_FLAG() system_time_flag_ &= ~0X04
#define CLEAN_TIME_1000MS_FLAG() system_time_flag_ &= ~0X08


void initSystemStat(void)
{
    system_stat_ = SYSTEM_AUTO_CTL;
    return;
}
void setSystemStat(unsigned char value)
{
    if ((value > 0 ) && (value < SYSTEM_STAT_END)) 
        system_stat_ = value ;
    return;
}
unsigned char getSystemStat(void)
{
    return system_stat_;
}
void initSystemErrorFlag(void)
{
    system_error_flag_ = 0;
    return;
}
void setSystemErrorFlag(unsigned char value)
{
    if ((value > 0 ) && (value < SYSTEM_ERROR_END)) 
        system_error_flag_ = value ;
    return;
}
unsigned char getSystemErrorFlag(void)
{
    return system_error_flag_;
}
void timerInit()
{  
  /***********************************************/
  //TODO: 
  //1-->> Capture/compare interrupt enable
  //2-->> set the time(1ms) jump into the interrupt
  //3-->> use SMCLK, upmode
  /***********************************************/
  CCR0 = 8000-1;                             // PWM Period  CCR1 = 500;                               // CCR1 PWM duty cycle
  TACTL = TASSEL_2 + MC_1;                  // SMCLK, up mode
  TACCTL0 = CCIE;
  CCR1 = 400-1;                             // PWM Period  CCR1 = 500;                               // CCR1 PWM duty cycle
  TACCTL1 = CCIE;
  _EINT(); 
}

void checkMCU()
{
  char i;
  char tmp;
  char flag;
  char *Flash_ptrD;   
  
  flag = 0;
  
  
  // Segment D pointer
  Flash_ptrD = (char *) 0x10E0;             // Initialize Flash segment D pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;  
  
  for (i = 0; i < 16; i++)
  {
    tmp = *Flash_ptrD;
    chip_id[i] = ~tmp;
    Flash_ptrD++;
  }
  
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  if ((chip_id_check[0] == chip_id_check[1]) && (chip_id_check[1] == chip_id_check[2]))
  {
   // tmp = 10;
   // pwm_out(0);
    Flash_ptrD = (char *)0XC000;                      //(char *)(&chip_id_check); OLD E000
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit
    //*Flash_ptrD = 0;                          // Dummy write to erase Flash segment D
    FCTL1 = FWKEY + WRT;  
    for (i = 0; i < 16; i++)
    {
      *Flash_ptrD = chip_id[i];
      Flash_ptrD++;
    }
    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  }
  else
  {
  //  for (i = 0; i < 16 ; i++)
    i = 0;
    while(i < 16)
    {
      tmp = chip_id_check[i];
      if (tmp != chip_id[i])
      {
        __delay_cycles(8000000);
        uart_tx_show_page(PAGE_JINGGAO);
       // bao_jing(1);
        __delay_cycles(800000);
        uart_tx_shangxian(80);
        while(1);
      }
      i++;
    }
  }
}
void keyScan(void)
{
    static const unsigned  char KEY_P1_PIN_MAP[] = {KEY_1_PIN,KEY_1_PIN};
    static const unsigned  char KEY_P2_PIN_MAP[] = {(KEY_3_PIN , KEY_4_PIN  ,KEY_5_PIN  ,KEY_6_PIN  ,XIAN_WEI_L_PIN  ,XIAN_WEI_R_PIN};
    char bit_count;
    char i;
    P1SEL &= ~(KEY_1_PIN | KEY_2_PIN);
    P2SEL &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_L_PIN | XIAN_WEI_R_PIN);
    P1DIR &= ~(KEY_1_PIN | KEY_2_PIN);
    P2DIR &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_R_PIN | XIAN_WEI_L_PIN);
    __delay_cycles(80);
    bit_count = 0;
    for (i = 0; i < 2; i++)
    {
        if((~P1IN) & KEY_1_PIN_MAP[i]))
            bit_count++;
    }
    for (i = 0; i < 4; i++)
    {
        if((~P2IN) & KEY_2_PIN_MAP[i]))
            bit_count++;
    }
    
    if (bit_count > 1)
    {
        setSystemErrorFlag(SYSTEM_ERROR_MANY_KEY_DOWN);    
        return ;
    }
    switch ((~P1IN) & (KEY_1_PIN | KEY_2_PIN))
    {
        case KEY_1_PIN : queue_add(&key_queue,KEY_POWER); break;
        case KEY_2_PIN : queue_add(&key_queue,KEY_SET); break;
        default: break;
    }
    switch ((~P2IN) & ( KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_R_PIN | XIAN_WEI_L_PIN ))
    {
        case KEY_3_PIN : queue_add(&key_queue,EVNET_KEY_UP); break;
        case KEY_4_PIN : queue_add(&key_queue,EVNET_KEY_DOWN); break;
        case KEY_5_PIN : queue_add(&key_queue,EVNET_KEY_OPEN); break;
        case KEY_6_PIN : queue_add(&key_queue,EVNET_KEY_CLOSE); break;
        case XIAN_WEI_L_PIN: queue_add(&key_queue,EVENT_LIMIT_COOL_ON); break;
        case XIAN_WEI_R_PIN: queue_add(&key_queue,EVENT_LIMIT_HEAT_ON); break;
        default: break;
    }
    return ;
}

void systemStatChange(unsigned char new_stat)
{
    unsigned char old_stat;
    if ((new_stat == 0) || (new_stat >= SYSTEM_STAT_END))
        return ;
    old_stat = getSystemStat();
    switch (new_stat)
    {
        case SYSTEM_AUTO_CTL : break;
        case SYSTEM_SETTING  : break;
        case SYSTEM_COOLING  : break;
        case SYSTEM_HEATING  : break;
        case SYSTEM_HANDLE   : break;
        case SYSTEM_WARNING  : break;
        case SYSTEM_POWEROFF : break;
        default break;
    }
    return;
}

void coolingHandle(void)
{
    
}
void tempAutoCtlHandle(void)
{

}

void systemInit(void )
{
    system_stat_ = 0;
    system_time_ = 0;
    system_time_flag_ = 0;
    system_error_flag_ = 0;
    limt_event_flag_ = 0;

    return ;
}


void taskFor10ms(void)
{
    keyScan();
}
void taskFor100ms(void)
{

}
void taskFor1000ms(void)
{

}

int main()
{
   queue_init(&key_queue);
   timerInit(); 
   while(1)
   {
       if(GET_TIME_10MS_FLAG())
       {
           taskFor10ms(void)
           CLEAN_TIME_10MS_FLAG();
       }
       if(GET_TIME_100MS_FLAG())
       {
           taskFor100ms(void)
           CLEAN_TIME_100MS_FLAG();
       }
       if(GET_TIME_1000MS_FLAG())
       {
           taskFor1000ms(void)
           CLEAN_TIME_1000MS_FLAG();
       }
   }

    return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Blinky(void)
{
    
  if (pwm_flag)
  {
    P1OUT |= PWM_BIT;
  }
  system_time_ ++;
  if (system_time_ >= 1000)
  {
    system_time_ = 0;
    SET_TIME_1000MS_FLAG();
  }
  else
  {
    if ((system_time_ % 10) == 0)
        SET_TIME_10S_FLAG();
    if ((system_time_ % 100) == 0)
        SET_TIME_100MS_FLAG();
  }  
}
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void) {
  switch( TA0IV )
	  {
	  case  2:                                  // CCR1
	    {
           if (pwm_flag) 
            P1OUT &= ~PWM_BIT;
	    }
	  break;
	  case  4: break;                           // CCR2 not used
	  case 10: break;                           // overflow not used
	 }
}


