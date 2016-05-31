#include "msp430g2332.h"
#include "config.h"
#include "queue.h"
#include "System.h"


static struct queue_type key_queue;
static unsigned char limt_event_flag_; 

unsigned char getLimtEventFlag(void)
{
    return limt_event_flag_;
}
void keyPinInit(void)
{
    P1SEL &= ~(KEY_1_PIN | KEY_2_PIN);
    P2SEL &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_L_PIN | XIAN_WEI_R_PIN);
    P1DIR &= ~(KEY_1_PIN | KEY_2_PIN);
    P2DIR &= ~(KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN | XIAN_WEI_R_PIN | XIAN_WEI_L_PIN);
}

void keyScan(void)
{
    static const unsigned  char KEY_P1_PIN_MAP[] = {KEY_1_PIN,KEY_1_PIN};
    static const unsigned  char KEY_P2_PIN_MAP[] = {(KEY_3_PIN , KEY_4_PIN  ,KEY_5_PIN  ,KEY_6_PIN  ,XIAN_WEI_L_PIN  ,XIAN_WEI_R_PIN};
    char bit_count;
    char i;
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
    switch ((~P2IN) & ( KEY_3_PIN | KEY_4_PIN | KEY_5_PIN | KEY_6_PIN  ))
    {
        case KEY_3_PIN : queue_add(&key_queue,EVNET_KEY_UP); break;
        case KEY_4_PIN : queue_add(&key_queue,EVNET_KEY_DOWN); break;
        case KEY_5_PIN : queue_add(&key_queue,EVNET_KEY_OPEN); break;
        case KEY_6_PIN : queue_add(&key_queue,EVNET_KEY_CLOSE); break;
        default: break;
    }
    if ((~P2IN) & (XIAN_WEI_L_PIN | XIAN_WEI_R_PIN))
    {
        if (((~P2IN) & (XIAN_WEI_L_PIN | XIAN_WEI_R_PIN)) == (XIAN_WEI_L_PIN | XIAN_WEI_R_PIN))
        {
            limt_event_flag_ = 0;
            //error 
        }
        else
        {
            if (~P2IN & XIAN_WEI_L_PIN) 
            {
                limt_event_flag_ = LIMT_EVENT_COOL_ON;
            }
            if (~P2IN & XIAN_WEI_R_PIN) 
            {
                limt_event_flag_ = LIMT_EVENT_HEAT_ON;
            }
        }
    }
    else
    {
        limt_event_flag_ = 0;
    }
    return ;
}

