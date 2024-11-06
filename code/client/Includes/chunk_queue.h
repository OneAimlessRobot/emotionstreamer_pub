#ifndef CHUNK_QUEUE_H
#define CHUNK_QUEUE_H

typedef struct chunk_queue{

	pthread_mutex_t* queue_mtx;
	uint32_t play_cursor,recv_cursor;
	uint8_t* chunk_buff;
	uint32_t n_occupied;
	uint32_t max_occupied;
	uint32_t chunk_size;
	uint32_t total_size;
	chunk_size_helper helper;
	
}chunk_queue;

int init_queue(chunk_queue* que,uint16_t chunk_size);

#endif
