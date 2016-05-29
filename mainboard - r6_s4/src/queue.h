#ifndef  __QUEUE_H__
#define __QUEUE_H__

#define QUEUE_LENTH     16
#define QUEUE_FULL      0X80
#define QUEUE_NULL      0X40
#define QUEUE_NUFULL    0X20
struct queue_type {
unsigned char queue_buffer[QUEUE_LENTH];
unsigned char queue_buffer_head_index;
unsigned char queue_buffer_tail_index;
unsigned char queue_flag;
};
char queue_init(struct queue_type *p);
char queue_add(struct queue_type *p,char vaule);
char queue_pop(struct queue_type *p,char * vaule);


#endif
