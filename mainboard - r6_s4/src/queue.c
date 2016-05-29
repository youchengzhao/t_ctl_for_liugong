#include "queue.h"


char queue_init(struct queue_type * p)
{
  p->queue_flag = QUEUE_NULL;
  p->queue_buffer_head_index = 0;
  p->queue_buffer_tail_index = 0;
  return 0;
}
char queue_add(struct queue_type *p, char vaule)
{
  if (p->queue_flag == QUEUE_FULL)
    return 0;
  p->queue_buffer[p->queue_buffer_head_index]= vaule;
  p->queue_buffer_head_index++;
  if (p->queue_buffer_head_index == QUEUE_LENTH)
  {
    p->queue_buffer_head_index = 0;
  }
  if (p->queue_flag == QUEUE_NULL)
  {
    p->queue_flag = QUEUE_NUFULL;
  }
  if (p->queue_buffer_head_index == p->queue_buffer_tail_index)
  {
    if (p->queue_flag == QUEUE_NUFULL)
    {
      p->queue_flag = QUEUE_FULL;
    }
  }
  return 1;
}
char queue_pop(struct queue_type *p,char * vaule)
{
  if (p->queue_flag == QUEUE_NULL)
  {
//    *vaule = 0;
    return 0;
  }
  if (p->queue_flag == QUEUE_NUFULL)
  {
    *vaule = p->queue_buffer[p->queue_buffer_tail_index];
    p->queue_buffer_tail_index++;
    if (p->queue_buffer_tail_index == QUEUE_LENTH)
    {
      p->queue_buffer_tail_index = 0;
    }
    if (p->queue_buffer_tail_index == p->queue_buffer_head_index)
    {
      p->queue_flag = QUEUE_NULL;
    }
  }
  if (p->queue_flag == QUEUE_FULL)
  {
    *vaule = p->queue_buffer[p->queue_buffer_tail_index];
    p->queue_buffer_tail_index++;
    if (p->queue_buffer_tail_index == QUEUE_LENTH)
    {
      p->queue_buffer_tail_index = 0;
    }
    p->queue_flag = QUEUE_NUFULL;
  }
  return 1;
}
