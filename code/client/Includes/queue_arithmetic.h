#ifndef QUEUE_ARITHMETIC_H
#define QUEUE_ARITHMETIC_H


void zero_chk_cache(chunk_queue* que);
uint64_t getQueueBufferedTime(chunk_queue* que, decoder_result_struct* result);
uint64_t circular_int_inc(uint64_t size,uint64_t a);
uint64_t circular_int_dec(uint64_t size,uint64_t a);
int que_is_empty(chunk_queue* que);
int que_is_full(chunk_queue* que);
int que_is_almost_empty(chunk_queue* que);
int que_is_almost_full(chunk_queue* que);
int que_pct_full(chunk_queue* que);
#endif
