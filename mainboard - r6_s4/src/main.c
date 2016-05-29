
//#include "io430.h"
#include "msp430g2332.h"
#define  DQ1		P1OUT |= BIT0
#define  DQ0		P1OUT &= ~BIT0
#define  DQ_in		P1DIR &= ~BIT0
#define  DQ_out		P1DIR |= BIT0
#define  DQ_val 	            (P1IN & BIT0) 

#define  UART_1		P1OUT |= BIT7
#define  UART_0		P1OUT &= ~BIT7
#define  UART_OUT		P1DIR |= BIT7

#define   PWM_BIT                   BIT3

#define UART_TBIT_DIV_2     (8000000 / (9600 * 2))
#define UART_TBIT           (8000000 / 9600)

#define SYSTEM_FLAG_AUTO            0X01
#define SYSTEM_FLAG_SETING           0X02
#define SYSTEM_FLAG_SEHNG_WEN       0X04
#define SYSTEM_FLAG_JIANG_WEN       0X08
#define SYSTEM_FLAG_HANDS           0X10
#define SYSTEM_FLAG_JING_GAO        0X20
#define SYSTEM_FLAG_POWER_OFF       0X40

#define SYS_TIME_FLAG_1S            0X01
#define SYS_TIME_FLAG_10MS          0X02
#define SYS_TIME_FLAG_100MS         0X04

#define KEY_SET                       0X01
#define KEY_POWER                     0X02
#define KEY_UP                        0X04
#define KEY_DOWN                      0X08
#define KEY_OPEN                      0X10
#define KEY_CLOSE                     0X20
#define KEY_1_PIN                   BIT4
#define KEY_2_PIN                   BIT5
#define KEY_3_PIN                   BIT0
#define KEY_4_PIN                   BIT1
#define KEY_5_PIN                   BIT2
#define KEY_6_PIN                   BIT3

#define XIAN_WEI_L_PIN                  BIT4
#define XIAN_WEI_R_PIN                  BIT5
#define XIAN_WEI_L                    0X40
#define XIAN_WEI_R                    0X80

#define RELAY_CTL_PIN               BIT1

#define BUZZER_PIN                  BIT6

#define DIAN_JI_DIAN_LIU_PIN        INCH_2
#define DIAN_JI_DIAN_LIU_MAX        30

//温度的缓冲控制，0.5度，当温度高于上限+0.2报警，温度回落到上限-0.3度报警结束
#define WENDU_BAO_JING_UP              102
#define WENDU_BAO_JING_DOWN              97

#define PAGE_AUTO                   0x44 
#define PAGE_SHENGWEN               0x45
#define PAGE_JANGWEN                0x46
#define PAGE_DAKAI                  0x47
#define PAGE_GUANBI                 0x48
#define PAGE_JINGGAO                0x49
#define PAGE_XIANWEI_JIANGWEN            0x4A
#define PAGE_XIANWEI_BACK                0x4B
#define PAGE_XIANWEI_SHENGWEN            0x4C

#define DIAN_JI_ZHUANG_XIANG_ZHENG  0X02
#define DIAN_JI_ZHUANG_XIANG_FAN  0X04


char t18b20_flag;
char system_flag;
char pwm_flag;
char sys_time_flag; char key_flag;
char power_off_flag ;
char xian_wei_flag;
char dian_ji_zhuang_xiang;
unsigned int dianji_dianliu;
unsigned int buzzer_ctl_count;
unsigned int pwm_duty;
unsigned int pwm_duty_show;
unsigned int sys_count; 
unsigned int shang_xian;
unsigned int xia_xian;
unsigned int shang_xian_h;
unsigned int xia_xian_h;
unsigned int shang_xian_l;
unsigned int xia_xian_l;
unsigned int shiji_wendu;
unsigned int system_open_count;
char chip_id[16];
static char chip_id_check[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                               0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,};
//static char chip_id_check[16]={0x01,0x02,0x03,0xff,0xff,0xff,0xff,0xff,
//                               0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,};

void uart_tx_show_page(char p);
void buzzer_ctl(unsigned int count);
void uart_tx_shangxian(int up);

//------------------------
void get_h_l_vaule()
{
    shang_xian_h = shang_xian + WENDU_BAO_JING_UP;
    shang_xian_l = shang_xian + WENDU_BAO_JING_DOWN;

    xia_xian_h = xia_xian + WENDU_BAO_JING_UP;
    xia_xian_l = xia_xian - WENDU_BAO_JING_DOWN;
}
//----------------------------ds18b20 driver-----------------------
char init_18b20()
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

void write_18b20(char data)
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
char read_18b20()
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
void skip()
{
    write_18b20(0xCC);
}
void convert()
{
  write_18b20(0x44);
}
void read_sp()
{
  write_18b20(0xbe);
}
unsigned int read_tmp()
{
  char low;
  unsigned int temp;
  low = read_18b20();
  temp = read_18b20();
  temp = (temp<<8) | low;
  return temp;
}
unsigned int do_convert()
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
     return value;
    }
}
//----------------------------ds18b20 driver-end--------------------

void uart_send_byte(char data)
{
  char i;
  _DINT();
  UART_0;
  __delay_cycles(UART_TBIT);
  UART_1;
  for (i = 0; i < 8; i++)
  {
    if(data & 0x01)
        UART_1;
    else
      UART_0;
     __delay_cycles(UART_TBIT);
     data >>=1;
  }
  UART_1;
  _EINT();
  __delay_cycles(UART_TBIT);
}

void Timer_Init()
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
void pwm_out(unsigned char duty)
{
  if (duty == 100)
  {
    pwm_duty_show=990;
  }
  else
  {
    pwm_duty_show = duty*10;
  }
  
  if(xian_wei_flag & XIAN_WEI_L)
  {
      if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_ZHENG)
      {
        pwm_flag = 0;
        P1OUT |= PWM_BIT;
        pwm_duty_show = 0;
         return;
      }
  }
  if(xian_wei_flag & XIAN_WEI_R)
  {
      if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_FAN)
      {
        pwm_flag = 0;
        P1OUT |= PWM_BIT;
        pwm_duty_show = 0;
        return;
      }
  }
  
  //pwm_flag = 1;
  if (duty == 100)
  {
      pwm_flag = 0;
      P1OUT &= ~PWM_BIT;
      return;
  }
  if(duty == 0)
  {
      pwm_flag = 0;
      P1OUT |= PWM_BIT;
      return;
  }
  if (duty > 100)
  {
    pwm_flag = 0;
    return;
  }
  CCR1 = 8000 - duty * 80;
  pwm_flag = 1;
    
}
/*********************   ADC  API    *****************************************/
void ADC_Reset(void){
  ADC10CTL0 &= ~ADC10ON; 
  ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON + REF2_5V + ADC10ON;  
}

void ADC_Off(void){
  ADC10CTL0 &= ~ADC10ON; 
//  ADC10CTL0 = 0;  
}

int Read_Voltage(int PinX_Voltage){
  int voltage=0;
  ADC_Reset();
  ADC10CTL1 = PinX_Voltage; 
  __delay_cycles(100);
  ADC10CTL0 |= ENC + ADC10SC;
 // __delay_cycles(5000);  
  while (ADC10CTL1 & ADC10BUSY);
  //__delay_cycles(5000);
  voltage = ADC10MEM; 
  //_NOP();
  ADC_Off();
  return voltage;
}

void dianji_dianliu_work()
{
    unsigned int tmp;
     tmp = Read_Voltage(DIAN_JI_DIAN_LIU_PIN); 
   //  tmp = 48;
  // i = (tmp /1024)*2.5 /0.5 *100 = tmp*5

    // dianji_dianliu = tmp*5;
     dianji_dianliu = tmp;
}

void read_flash_data()
{
  char tmp;
  char *Flash_ptrD;                         // Segment D pointer
  Flash_ptrD = (char *) 0x1000;             // Initialize Flash segment D pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;  
  
  shang_xian = 0;
  tmp = *Flash_ptrD;
  shang_xian = tmp<<8;
  Flash_ptrD++;
  tmp = *Flash_ptrD;
  shang_xian |= tmp;
  
  xia_xian = 0;
  Flash_ptrD++;
  tmp = *Flash_ptrD;
  xia_xian = tmp<<8;
  Flash_ptrD++;
  tmp =  *Flash_ptrD;
  xia_xian |= tmp;

  
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}
void write_flash_data()
{
  char tmp;
    char *Flash_ptrD;                         // Segment D pointer
  Flash_ptrD = (char *) 0x1000;             // Initialize Flash segment D pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  *Flash_ptrD = 0;                          // Dummy write to erase Flash segment D
  FCTL1 = FWKEY + WRT;  
  
  tmp = (shang_xian>>8) & 0XFF;
  *Flash_ptrD = tmp;
  Flash_ptrD++;
  tmp = shang_xian & 0xff;
  *Flash_ptrD = tmp;
  
  
  Flash_ptrD++;
  tmp = (xia_xian>>8) & 0XFF;
  *Flash_ptrD = tmp;
  Flash_ptrD++;
  tmp = xia_xian & 0xff;
  *Flash_ptrD = tmp;
  
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void check_mcu()
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
//  if (0)
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

char key_power_flag = 0;
//----- 按键扫描--------------------
void scan_key()
{
    // P1 ->  KEY_1_PIN | KEY_2_PIN
    // P2 ->  KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN
    P1SEL &= ~(KEY_1_PIN | KEY_2_PIN);
    P2SEL &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_L_PIN | XIAN_WEI_R_PIN);
    P1DIR &= ~(KEY_1_PIN | KEY_2_PIN);
    P2DIR &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_R_PIN | XIAN_WEI_L_PIN);
    key_flag = 0;
    if (P1IN & KEY_1_PIN)
    {
        //key_flag &= ~KEY_POWER;
        if (key_power_flag == 1)
          key_flag |= KEY_POWER;
            ///key_power_flag = 1;
    }
    else
    {
       // key_flag |= KEY_POWER;
      if (key_power_flag == 0)
          key_power_flag = 1;
    }

    if (P1IN & KEY_2_PIN)
        key_flag &= ~KEY_SET;
    else
        key_flag |= KEY_SET;

    if (P2IN & KEY_3_PIN)
        key_flag &= ~KEY_UP;
    else
        key_flag |= KEY_UP;
    
    if (P2IN & KEY_4_PIN)
        key_flag &= ~KEY_DOWN;
    else
        key_flag |= KEY_DOWN;

    if (P2IN & KEY_5_PIN)
        key_flag &= ~KEY_OPEN;
    else
        key_flag |= KEY_OPEN;

    if (P2IN & KEY_6_PIN)
        key_flag &= ~KEY_CLOSE;
    else
        key_flag |= KEY_CLOSE;

    if (!(P2IN & XIAN_WEI_L_PIN))
        xian_wei_flag |= XIAN_WEI_L;
    else
        xian_wei_flag &= ~XIAN_WEI_L;

    if (!(P2IN & XIAN_WEI_R_PIN))
        xian_wei_flag |= XIAN_WEI_R;
    else
        xian_wei_flag &= ~XIAN_WEI_R;
        
   if (key_flag != 0)
       P2OUT |= BUZZER_PIN;
   else
      P2OUT &= ~BUZZER_PIN;

    
    return;
}
//----- 按键扫描--------------------
char open_flag = 0;
char close_flag = 0;
 unsigned int handle_run_count_50 = 0;
void key_to_sys_flag()
{
    static char power_flag = 0;

  //  static int set_count = 0;
    static int open_count = 0;
    static int close_count = 0;
    
    if (key_flag & (XIAN_WEI_L_PIN | XIAN_WEI_R_PIN)) //当出现报警的情况也有清除
    {
        open_flag = 0; //按下其他的键 取消手动标志
        close_flag = 0;
    }
    
    if (key_flag & KEY_POWER)    
    {
        open_flag = 0; //按下其他的键 取消手动标志
        close_flag = 0;
        
        key_flag &= ~KEY_POWER;  //清除标志
        key_power_flag = 0;
        
        if (power_flag !=0) 
            return;
        if (power_off_flag == 0)
        {
            pwm_out(0);
            buzzer_ctl(100);
            power_off_flag = 1;
            system_flag = SYSTEM_FLAG_POWER_OFF;
            uart_tx_show_page(0x50);
        }
        else
        {
            uart_tx_show_page(PAGE_AUTO);
            read_flash_data();
            shiji_wendu = xia_xian + (shang_xian-xia_xian)/2;
            system_flag = SYSTEM_FLAG_AUTO;
            power_off_flag = 0;
            __delay_cycles(100);
            WDTCTL = WDTPW + WDTIS1;
            
        }
        power_flag = 1;
        return ;        
    }
    else
    {
        power_flag = 0;
    }
    if (system_flag & SYSTEM_FLAG_POWER_OFF )
          return ;
    if (system_flag & SYSTEM_FLAG_JING_GAO )
          return ;
    if (key_flag & KEY_OPEN)    
    {
        if (open_flag == 1)
        {
            return;
        }
        open_count ++;
        if (open_count >= 20) // open key 按下2S 进入手动打开状态
        {
            open_flag = 1;
            handle_run_count_50 = 0;
            system_flag = SYSTEM_FLAG_HANDS;
            open_count = 22; 
            uart_tx_show_page(PAGE_DAKAI);
        }
        return ;        
    }
    else
    {
        open_count = 0;
        /*   //这个地方放到按下按键要走5分钟的流程去解决  在这个函数里 sys_handle_run
        if (open_flag == 1)
        {
            system_flag = SYSTEM_FLAG_AUTO;
            uart_tx_show_page(PAGE_AUTO);
        }
        open_flag = 0;
        */
    }

    if (key_flag & KEY_CLOSE)    
    {
        if (close_flag == 1)
        {
            return;
        }
        close_count ++;
        if (close_count >= 20) // open key 按下2S 进入手动打开状态
        {
            close_flag = 1;
            system_flag = SYSTEM_FLAG_HANDS;
            close_count = 22; 
            uart_tx_show_page(PAGE_GUANBI);
        }
        return ;        
    }
    else
    {
        close_count = 0;
        /*   //这个地方放到按下按键要走5分钟的流程去解决  在这个函数里 sys_handle_run
        if (close_flag == 1)
        {
            system_flag = SYSTEM_FLAG_AUTO;
            uart_tx_show_page(PAGE_AUTO);
        }
        close_flag = 0;
        */
    }


    if (key_flag & KEY_SET)    
    {
        open_flag = 0; //按下其他的键 取消手动标志
        close_flag = 0;
        if (system_flag & SYSTEM_FLAG_POWER_OFF)
          return ;
        if (!(system_flag & SYSTEM_FLAG_SETING))
            uart_tx_show_page(PAGE_AUTO);
        system_flag = SYSTEM_FLAG_SETING;
        return ;        
    }
    else
    {
        key_flag &= ~KEY_SET;
    }
}
void xian_wei_bao_hu()
{
    if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_ZHENG)
    {
        if (xian_wei_flag & XIAN_WEI_L) 
        {
          pwm_out(0);
        }
    }
    if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_FAN)
    {
        if (xian_wei_flag & XIAN_WEI_R) 
        {
          pwm_out(0);
        }

    }
}
void bao_jing(char cmd)
{
  P2SEL &= ~BIT7;
  P2DIR |= BIT7;
  if (cmd == 0)
      P2OUT |= BIT7;
  else
      P2OUT &= ~BIT7;
}
//-----
void dianji_zheng()
{
    P1SEL &= ~RELAY_CTL_PIN;
    P1DIR |= RELAY_CTL_PIN;
    P1OUT |= RELAY_CTL_PIN;
    dian_ji_zhuang_xiang = DIAN_JI_ZHUANG_XIANG_ZHENG;
}
void dianji_fan()
{
    P1SEL &= ~RELAY_CTL_PIN;
    P1DIR |= RELAY_CTL_PIN;
    P1OUT &= ~RELAY_CTL_PIN;
    dian_ji_zhuang_xiang = DIAN_JI_ZHUANG_XIANG_FAN;
}


//----------------------------设置蜂鸣器响的时间，单位10ms--------------------
void buzzer_ctl(unsigned int count)
{
    buzzer_ctl_count = count;
}

//----------------------------蜂鸣器响控制--------------------
void buzzer_init()
{
    P2SEL &= ~BUZZER_PIN;
    P2DIR |= BUZZER_PIN;
    P2OUT &= ~BUZZER_PIN;
}
//----------------------------蜂鸣器响控制--------------------
void buzzer_work()
{

  if (buzzer_ctl_count == 0)
      return;
  if (buzzer_ctl_count > 1)
    {
        buzzer_ctl_count--;
        P2OUT |= BUZZER_PIN;
    }
    if (buzzer_ctl_count == 1)
    {
        P2OUT &= ~BUZZER_PIN;
        buzzer_ctl_count--;
    }
}


//----------------------------自动运行--------------------
void sys_auto_run()
{
  pwm_out(0);
  if (shiji_wendu > shang_xian)
  {
    system_flag = SYSTEM_FLAG_JIANG_WEN;
    uart_tx_show_page(PAGE_JANGWEN);
    __delay_cycles(800);
  }
  //if ((shiji_wendu < (shang_xian-WENDU_BAO_JING_UP)) && (shiji_wendu > (shang_xian+WENDU_BAO_JING_DOWN)))
  if((shiji_wendu < shang_xian_l) && (shiji_wendu > xia_xian_h))
  {
    bao_jing(0);
    dianji_zheng(); 
  }
  if (shiji_wendu < xia_xian)
  {
    system_flag = SYSTEM_FLAG_SEHNG_WEN;
    dian_ji_zhuang_xiang = 0;
    uart_tx_show_page(PAGE_SHENGWEN);
    __delay_cycles(8000);
  }
}


void sys_sheng_jiang_wen(int a,int b)
{
  if ((a - b) < 2)//0.2度
  {
    pwm_out(0);
   // pwm_duty_show = 0;
    system_flag = SYSTEM_FLAG_AUTO;
    uart_tx_show_page(PAGE_AUTO);
  }
  pwm_out(100);
  if ((a - b) <5)// < 0.5度
  {   
    pwm_out(50);
    return;
  }
   if (((a - b) >=5) && ((a - b)< 10))// 0.5<= tmp < 1度
  {   
    pwm_out(80);
    return;
  }
   if ((a - b) > 10)// < 0.5度
  {   
    pwm_out(100);
    return;
  } 

}
//----------------------------升温--------------------
void sys_sheng_wen_run()
{
  unsigned int tmp;
  tmp = shang_xian-xia_xian;
  tmp = tmp/2;
  if (tmp >= 5)
    tmp = 5;
  tmp += xia_xian ;
  
  dianji_zheng();
//  if (shiji_wendu < (xia_xian -WENDU_BAO_JING_UP))
  if (shiji_wendu < (xia_xian_l))
  {
    bao_jing(0);
  }
//  if (shiji_wendu > (xia_xian + WENDU_BAO_JING_DOWN))
  if (shiji_wendu > (xia_xian_h ))
  {
    bao_jing(0);
  }
    sys_sheng_jiang_wen(tmp,shiji_wendu);
 /* 
  if ((tmp - shiji_wendu) < 2)//0.2度
  {
    pwm_out(0);
    pwm_duty_show = 0;
    system_flag = SYSTEM_FLAG_AUTO;
    uart_tx_show_page(PAGE_AUTO);
  }
  
  if ((tmp - shiji_wendu) <5)// < 0.5度
  {   
    pwm_out(50);
    pwm_duty_show = 500;
    return;
  }
   if (((tmp - shiji_wendu) >=5) && ((tmp - shiji_wendu)< 10))// 0.5<= tmp < 1度
  {   
    pwm_out(80);
    pwm_duty_show = 800;
    return;
  }
   if ((tmp - shiji_wendu) > 10)// < 0.5度
  {   
    pwm_out(100);
    pwm_duty_show = 990;
    return;
  } 
  */
}
//---------------------------降温--------------------
void sys_jiang_wen_run()
{
  unsigned int tmp;
  tmp = shang_xian-xia_xian;
  tmp = tmp/2;
  //tmp = shang_xian-tmp;
   if (tmp >= 5)
    tmp = 5;
  tmp = shang_xian-tmp;
  
  dianji_fan();
//  if (shiji_wendu > (shang_xian +WENDU_BAO_JING_UP))
  if (shiji_wendu > (shang_xian_h ))
  {
    bao_jing(1);
    
  }
  if (shiji_wendu < (shang_xian_l ))
  {
    bao_jing(0);
  }
sys_sheng_jiang_wen(shiji_wendu,tmp);
/*  
  if ((shiji_wendu - tmp) < 2)//0.2度
  {
    pwm_out(0);
    system_flag = SYSTEM_FLAG_AUTO;
    uart_tx_show_page(PAGE_AUTO);
  }
  
  if ((shiji_wendu - tmp) <5)// < 0.5度
  {   
    pwm_out(50);
    return;
  }
   if (((shiji_wendu - tmp) >=5) && ((shiji_wendu - tmp)< 10))// 0.5<= tmp < 1度
  {   
    pwm_out(80);
    return;
  }
   if ((shiji_wendu  -tmp) > 10)// < 0.5度
  {   
    pwm_out(100);
    return;
  } 
  */
}

//----------------------------设置--------------------
void sys_seting_run()
{
  static int count = 0;
  static int count_chage = 0;
  static char selet_flag = 0;
  static char key_down_flag = 0;
  char tmpa;
  char tmpb;
  char tmpc;
 
  
  if (!(system_flag & SYSTEM_FLAG_SETING))
      return;
  if (key_flag & KEY_SET)  //设置上下限
  {
    if (!(key_down_flag & 0x01))
    {
        count_chage  = 0;
        selet_flag ^= 0x01;
        key_down_flag |= 0x01;
    }
  }
  else
  {
        key_down_flag &= ~0x01;
  }
  count ++;
  count_chage++;
  //----------0.5S显示闪烁
  if (count == 50)
  {
     tmpa = shang_xian/100;
     tmpb = (shang_xian%100)/10;
     tmpc = 0x42;
     uart_send_byte(0x85);
     uart_send_byte(0x42);
     uart_send_byte(tmpa);
     uart_send_byte(tmpb);
     uart_send_byte(0x00);
     uart_send_byte(0x00);

     tmpa = xia_xian/100;
     tmpb = (xia_xian%100)/10;
     tmpc = 0x43;
     uart_send_byte(0x85);
     uart_send_byte(0x43);
     uart_send_byte(tmpa);
     uart_send_byte(tmpb);
     uart_send_byte(0x00);
     uart_send_byte(0x00);
  } 
 if (count >= 100)
  {
     if(selet_flag == 0)
     {
       tmpa = shang_xian/100;
       tmpc = 0x42;
     }
     else
     {
       tmpa = xia_xian/100;
       tmpc = 0x43;
     }
     tmpb = 10;
     uart_send_byte(0x85);
     uart_send_byte(tmpc);
     uart_send_byte(tmpa);
     uart_send_byte(tmpb);
     uart_send_byte(0x00);
     uart_send_byte(0x00);
     count = 0;
  } 
  
  //-----3s后没有按键自动退出
  if (count_chage >= 500)
  {
    count_chage = 0;
    selet_flag = 0;
    system_flag = SYSTEM_FLAG_AUTO;
    write_flash_data();
    buzzer_ctl(300);
    uart_tx_show_page(PAGE_AUTO);
    get_h_l_vaule();

    return ;
  }
 if (key_flag & KEY_UP) //往上加
 {
    count_chage = 0;
    if (!(key_down_flag & 0x02))
    {
        key_down_flag |= 0x02;
        if (selet_flag == 0)
        { 
          if ((shang_xian - xia_xian) < 80)//40
           shang_xian += 10;
        }
        else
        {
          if ((shang_xian - xia_xian) > 10 )
             xia_xian += 10;
        }
        
    }
 }
 else
 {
     key_down_flag &= ~ 0x02;
 }
 if (key_flag & KEY_DOWN)//往下减
 {
    count_chage = 0;
    if (!(key_down_flag & 0x04))
    {
        key_down_flag |= 0x04;
        if (selet_flag == 0)
        { 
          if ((shang_xian - xia_xian) > 10)
           shang_xian -= 10;
        }
        else
        {
          if ((shang_xian - xia_xian) < 80)//40
             xia_xian -= 10;
        }
        
    }
 }
 else
 {
     key_down_flag &= ~ 0x04;
 }

}
//---------------------------手动---------------
void sys_handle_run()
{
  
//  static unsigned int count_2 = 0;
  //static char run_flag = 0;
  if (open_flag == 1)
  {
    handle_run_count_50++;
      dianji_fan();
      if (!(xian_wei_flag & XIAN_WEI_R))
      {
          pwm_out(100);
          
      }
      else 
      {
          pwm_out(0);
           __delay_cycles(8000);
          // uart_tx_show_page(PAGE_XIANWEI);
      }
  }
  
  if (close_flag == 1)
  {
    handle_run_count_50++;
    dianji_zheng();
    if (!(xian_wei_flag & XIAN_WEI_L))
    {
        pwm_out(100);
    }
    else 
    {
        pwm_out(0);
         __delay_cycles(8000);
         //  uart_tx_show_page(PAGE_XIANWEI);
    }
  }
  
    if (handle_run_count_50 > 900 )  //5min  = 5x60 = 300, 15min = 15x60 900
    {
      handle_run_count_50 = 0;
      open_flag = 0;
      close_flag = 0;
      system_flag = SYSTEM_FLAG_AUTO;
      uart_tx_show_page(PAGE_AUTO);
    }
}


void system_chage()
{
   // static int count = 0;
    switch (system_flag)
    {
    case SYSTEM_FLAG_AUTO        :  sys_auto_run(); break;
    case SYSTEM_FLAG_SETING      :  break;
    case SYSTEM_FLAG_SEHNG_WEN   : sys_sheng_wen_run();break;
    case SYSTEM_FLAG_JIANG_WEN   : sys_jiang_wen_run();break;
    case SYSTEM_FLAG_HANDS       :  sys_handle_run(); break; 
    case SYSTEM_FLAG_JING_GAO    :  
    case SYSTEM_FLAG_POWER_OFF   : break;  
    }
}

void uart_tx_show_page( char p )
{
    uart_send_byte(0x85);
    uart_send_byte(p);
    uart_send_byte(0x00);
    uart_send_byte(0x00);
    uart_send_byte(0x00);
    uart_send_byte(0x00);
}
void uart_tx_shangxian(int up)
{
    char a,b;
    int ttmp = up;
    a = ttmp/100;
    b = (ttmp%100)/10;
    uart_send_byte(0x85);
    uart_send_byte(0x42);
    uart_send_byte(a);
    uart_send_byte(b);
    uart_send_byte(0x00);
    uart_send_byte(0x00);
}
void uart_tx_xiaxian(int down)
{
    char a,b;
    int ttmp = down;
    a = ttmp/100;
    b = (ttmp%100)/10;
    uart_send_byte(0x85);
    uart_send_byte(0x43);
    uart_send_byte(a);
    uart_send_byte(b);
    uart_send_byte(0x00);
    uart_send_byte(0x00);
}
void uart_tx_shijiwendu()
{
    char a,b,c;
    int ttmp = shiji_wendu;
    a = ttmp/100;
    b = (ttmp%100)/10;
    c = ttmp %10;
    uart_send_byte(0x85);
    uart_send_byte(0x41);
    uart_send_byte(a);
    uart_send_byte(b);
    uart_send_byte(c);
    uart_send_byte(0x00);
}
void uart_tx_dianjidianliu()
{
    char a,b,c;
    int ttmp = dianji_dianliu;
    a = ttmp/100;
    b = (ttmp%100)/10;
    c = ttmp %10;
    uart_send_byte(0x85);
    uart_send_byte(0x4D);
    uart_send_byte(a);
    uart_send_byte(b);
    uart_send_byte(c);
    uart_send_byte(0x00);
}

void task_for_1s()
{
  
  unsigned int tmp;
  static int dianji_dianliu_count = 0;
  static int sys_handle_dianjidianliu_count = 0;
  static char wen_du_bao_jing = 0;
  static char shengwen_flag = 0;
  static char jianggwen_flag = 0;
  if (!(sys_time_flag & SYS_TIME_FLAG_1S))
  {
    return;
  }
  sys_time_flag &= ~SYS_TIME_FLAG_1S;
  if (system_flag & SYSTEM_FLAG_POWER_OFF)
    return;
 
  system_chage();
  tmp = do_convert();
  if(tmp > 1)
      shiji_wendu = tmp;

   dianji_dianliu_work();
  // dianji_dianliu = 50;
   
  dianji_dianliu = dianji_dianliu/10;
  //dianji_dianliu +=3;
  if (system_flag & SYSTEM_FLAG_AUTO)
  {
      
    uart_tx_xiaxian(xia_xian);
    uart_tx_shangxian(shang_xian);
    uart_tx_shijiwendu();
  //   if (wen_du_bao_jing > 1)    //r04 修改
       wen_du_bao_jing = 0;
    
  }
  if (system_flag & SYSTEM_FLAG_SEHNG_WEN) //-------------------升温
  {
    uart_tx_shangxian(pwm_duty_show);
  //  uart_tx_xiaxian(dianji_dianliu);
    __delay_cycles(8000);
    uart_tx_shijiwendu();
    __delay_cycles(8000);
    if (pwm_duty_show == 0) 
    {
      __delay_cycles(8000);
      uart_tx_show_page(PAGE_XIANWEI_SHENGWEN);
      __delay_cycles(8000);
      shengwen_flag = 1;   
    }
    else
    {
     // uart_tx_show_page(PAGE_XIANWEI_BACK);
      if(shengwen_flag == 1)
      {
        shengwen_flag = 0;
        uart_tx_show_page(PAGE_SHENGWEN);
        uart_tx_show_page(PAGE_XIANWEI_BACK);
      }
       __delay_cycles(8000);
      uart_tx_dianjidianliu();
      __delay_cycles(8000);
    }
   
  }
  if (system_flag & SYSTEM_FLAG_JIANG_WEN) //----------------降温
  {
    if (shiji_wendu > (shang_xian_h ))
    {
      //if (wen_du_bao_jing == 0)
        //  wen_du_bao_jing = 1;
       if (wen_du_bao_jing == 0)
       {
         __delay_cycles(8000000);
          uart_tx_show_page(PAGE_JINGGAO); //温度超标显示报警2
          __delay_cycles(800000);
          uart_tx_shangxian(20);
       }
       else
       {
         uart_tx_shangxian(20);
       }
       wen_du_bao_jing = 10;
    }
    else
    {
      if (wen_du_bao_jing > 1)
      {
        wen_du_bao_jing = 0;
        __delay_cycles(8000);
        uart_tx_show_page(PAGE_JANGWEN);
        __delay_cycles(8000);
      }
      __delay_cycles(8000);
      uart_tx_shangxian(pwm_duty_show);
      __delay_cycles(8000);
      if (pwm_duty_show == 0)
      {
        jianggwen_flag = 1;
        __delay_cycles(8000);
        uart_tx_shijiwendu();
        __delay_cycles(8000);
        uart_tx_show_page(PAGE_XIANWEI_JIANGWEN);
        __delay_cycles(8000);
      }
      else  
      {
        if (jianggwen_flag == 1)
        {
          jianggwen_flag = 0;
          __delay_cycles(8000);
          uart_tx_show_page(PAGE_JANGWEN);
          __delay_cycles(8000);
        }
        __delay_cycles(8000);
        uart_tx_show_page(PAGE_XIANWEI_BACK);
        __delay_cycles(8000);
       uart_tx_shijiwendu();
       __delay_cycles(8000);
       uart_tx_dianjidianliu();
      }
      
    }
  }
  else
  {
    wen_du_bao_jing = 0;
  }
   if (system_flag & SYSTEM_FLAG_HANDS)  //--------------手动
  {

    uart_tx_shangxian(pwm_duty_show);
    tmp = shiji_wendu;
    if (pwm_duty_show == 0)
      dianji_dianliu = 1;
    shiji_wendu = dianji_dianliu;
    __delay_cycles(8000);
    uart_tx_shijiwendu();
    __delay_cycles(8000);
    shiji_wendu = tmp;
   // uart_tx_xiaxian(dianji_dianliu);
    if (pwm_duty_show == 0)
    {
      dianji_dianliu = 10;
      if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_ZHENG)
          uart_tx_show_page(PAGE_XIANWEI_SHENGWEN);
      if (dian_ji_zhuang_xiang & DIAN_JI_ZHUANG_XIANG_FAN)       
          uart_tx_show_page(PAGE_XIANWEI_JIANGWEN);
    }
    else
    {
      uart_tx_show_page(PAGE_XIANWEI_BACK);
    }

    
    if(dianji_dianliu > DIAN_JI_DIAN_LIU_MAX)
    {
      sys_handle_dianjidianliu_count++;
      if (sys_handle_dianjidianliu_count >= 3)
      {
        pwm_out(0);
        sys_handle_dianjidianliu_count = 20;
      }
    }
    else
    {
      sys_handle_dianjidianliu_count = 0;
    }
  } 
  
  if (system_flag & SYSTEM_FLAG_SETING)  //--------------设置
  {
    uart_tx_shijiwendu();
  }
  
   if (xian_wei_flag == (XIAN_WEI_L | XIAN_WEI_R)) //------报警
   { 
        if(!(system_flag & SYSTEM_FLAG_JING_GAO))
        {
            system_flag = SYSTEM_FLAG_JING_GAO;
            __delay_cycles(800000);
            uart_tx_show_page(PAGE_JINGGAO);
            bao_jing(1);
            __delay_cycles(800000);
            uart_tx_shangxian(10);
        }
   }
   else
   {
     if (dianji_dianliu_count < 4)
     {
        if(system_flag & SYSTEM_FLAG_JING_GAO)
        {
            system_flag = SYSTEM_FLAG_AUTO;
            uart_tx_show_page(PAGE_AUTO);
            bao_jing(0);
            
        }
     }
   }
  
  if (dianji_dianliu >= DIAN_JI_DIAN_LIU_MAX)
  {
    dianji_dianliu_count++;
  }
  else
  {
    if (dianji_dianliu_count < 4) 
      dianji_dianliu_count = 0;
    else 
      dianji_dianliu_count++;
  }
  if (dianji_dianliu_count == 4)
  {
    system_flag = SYSTEM_FLAG_JING_GAO;
    __delay_cycles(8000);
    uart_tx_show_page(PAGE_JINGGAO);
    bao_jing(1);
    __delay_cycles(800000);
    uart_tx_shangxian(30);
    pwm_out(0);
  }
  if (dianji_dianliu_count > 300)
  {
    dianji_dianliu_count = 0;
    system_flag = SYSTEM_FLAG_AUTO;
    uart_tx_show_page(PAGE_AUTO);
    bao_jing(0);
  }
  return;
}
void task_for_10ms()
{
  if (!(sys_time_flag & SYS_TIME_FLAG_10MS))
  {
    return;
  }
  sys_time_flag &= ~SYS_TIME_FLAG_10MS;
  
  scan_key();
  sys_seting_run();
  xian_wei_bao_hu();
  buzzer_work();
  return;
}
void task_for_100ms()
{
  if (!(sys_time_flag & SYS_TIME_FLAG_100MS))
  {
    return;
  }
  sys_time_flag &= ~SYS_TIME_FLAG_100MS;
 // dianji_dianliu_work();
  key_to_sys_flag();
    return;
}
/*
void xian_wei_kai_guan_done()
{
  if (system_flag & (SYSTEM_FLAG_SEHNG_WEN | SYSTEM_FLAG_JIANG_WEN | SYSTEM_FLAG_HANDS ))
  {
    if (xian_wei_flag & (XIAN_WEI_L |XIAN_WEI_R )) 
    {
        __delay_cycles(8000);
        uart_tx_show_page(0x4A);
    }
  }
}
*/


void system_init()
{
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_8MHZ;     //这个型号内部晶振只有 1MHz，8MHz，12MHz，16MHz，没有4MHz，要使用4MHz，只能进行对相关时钟进行 2 分频。               
  DCOCTL = CALDCO_8MHZ;
  init_18b20();
  UART_OUT;
  pwm_flag = 0;
  P1SEL &= ~PWM_BIT;
  P1DIR |= PWM_BIT;
  P1OUT &= ~PWM_BIT;

  sys_count = 0; 
  key_flag = 0;
  buzzer_ctl_count = 0;
  system_flag = 0;

  power_off_flag = 0;
  dian_ji_zhuang_xiang = 0;

  xian_wei_flag = 0;

  Timer_Init();
  buzzer_init();
  bao_jing(0);
    
}
int main( void )
{
//  char a,b,c;

  // Stop watchdog timer to prevent time out reset
  read_flash_data();
  if (shang_xian == xia_xian)
  {
    shang_xian = 310;
    xia_xian = 270;
    write_flash_data();
  }
  get_h_l_vaule();
  shiji_wendu = xia_xian + (shang_xian - xia_xian)/2;
  system_init();
  system_flag = SYSTEM_FLAG_AUTO;
  
  uart_tx_show_page(PAGE_AUTO);
  
  __delay_cycles(8000);
  check_mcu();
  pwm_out(50);
  while(1)
  {
    task_for_10ms();
    task_for_100ms();
    if (power_off_flag == 1)
        continue;
   
    task_for_1s();
  }
  //return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Blinky(void)
{
    
  if (pwm_flag)
  {
    P1OUT |= PWM_BIT;
  }
  sys_count++;
  if (sys_count >= 1000)
  {
    sys_count = 0;
    sys_time_flag |= SYS_TIME_FLAG_1S;
  }
  else
  {
    if ((sys_count % 10) == 0)
        sys_time_flag |= SYS_TIME_FLAG_10MS;
    if ((sys_count % 100) == 0)
        sys_time_flag |= SYS_TIME_FLAG_100MS;
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
