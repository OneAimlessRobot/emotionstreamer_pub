#ifndef CHUNK_QUEUE_H
#define CHUNK_QUEUE_H

typedef struct chunk_queue{

	uint64_t play_cursor;
	uint64_t recv_cursor;
	uint64_t n_occupied;
	uint64_t max_occupied;
	uint64_t chunk_size;
	uint64_t total_size;
	uint64_t display_size;
	pthread_mutex_t* queue_mtx;
	uint8_t* chunk_buff;
	chunk_size_helper helper;
	char* queue_name;
}chunk_queue;

int init_queue(chunk_queue* que,uint64_t chunk_size,uint64_t max_occupied,uint64_t display_size,char* queue_name);

#endif
