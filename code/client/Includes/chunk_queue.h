#ifndef CHUNK_QUEUE_H
#define CHUNK_QUEUE_H

typedef struct chunk_queue{

	uint32_t play_cursor;
	uint32_t recv_cursor;
	uint32_t n_occupied;
	uint32_t max_occupied;
	uint32_t chunk_size;
	uint32_t total_size;
	uint32_t sub_chunk_occupied;
	pthread_mutex_t* queue_mtx;
	uint8_t* chunk_buff;
	chunk_size_helper helper;
	
}chunk_queue;

int init_queue(chunk_queue* que,uint64_t chunk_size);

#endif
