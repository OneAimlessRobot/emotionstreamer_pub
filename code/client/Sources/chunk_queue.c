#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_arithmetic.h"
#include "../Includes/queue_menus.h"

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;


int init_queue(chunk_queue* que,uint16_t chunk_size){

	que->queue_mtx=&mtx;
	que->play_cursor=0;
	que->recv_cursor=0;
	que->n_occupied=0;
	que->chunk_size=chunk_size;
	que->max_occupied=cfg_stream_cache_size_chunks;
	que->total_size=que->max_occupied*que->chunk_size;
	que->chunk_buff=malloc(que->total_size);
	memset(que->chunk_buff,0,que->total_size);
	return 1;
}
