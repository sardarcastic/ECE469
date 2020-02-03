#ifndef __USERPROG__
#define __USERPROG__

typedef struct circular_buffer {
  int head;
  int tail;
  char * buff; 
} circular_buffer;

#define PRODUCER_TO_RUN "producer.dlx.obj"
#define CONSUMER_TO_RUN "consumer.dlx.obj"

#endif
