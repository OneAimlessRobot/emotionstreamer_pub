#ifndef QUEUE_ARITHMETIC_H
#define QUEUE_ARITHMETIC_H

#define PRINT_SIZE 300


void zero_chk_cache(chunk_queue* que);
uint32_t getQueueBufferedTime(chunk_queue* que);
uint32_t circular_int_inc(uint32_t size,uint32_t a);
uint32_t circular_int_dec(uint32_t size,uint32_t a);
int que_is_empty(chunk_queue* que);
int que_is_full(chunk_queue* que);
int que_is_almost_empty(chunk_queue* que);
int que_is_almost_full(chunk_queue* que);
int que_pct_full(chunk_queue* que);
#endif
