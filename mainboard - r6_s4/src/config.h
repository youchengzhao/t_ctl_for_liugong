#include "msp430g2553.h"
#define KEY_1_PIN                   BIT4
#define KEY_2_PIN                   BIT5
#define KEY_3_PIN                   BIT0
#define KEY_4_PIN                   BIT1
#define KEY_5_PIN                   BIT2
#define KEY_6_PIN                   BIT3

#define XIAN_WEI_L_PIN                  BIT4
#define XIAN_WEI_R_PIN                  BIT5

#define RELAY_CTL_PIN               BIT1

#define BUZZER_PIN                  BIT6

#define DIAN_JI_DIAN_LIU_PIN        INCH_2

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


