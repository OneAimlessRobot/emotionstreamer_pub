#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_arithmetic.h"
#include "../Includes/queue_menus.h"

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;


int init_queue(chunk_queue* que,uint64_t chunk_size,uint64_t max_occupied,uint64_t display_size,char* queue_name){

	que->queue_mtx=&mtx;
	que->play_cursor=0;
	que->recv_cursor=0;
	que->n_occupied=0;
	que->chunk_size=chunk_size;
	que->display_size=display_size;
	que->max_occupied=max_occupied;
	que->total_size=que->max_occupied*que->chunk_size;
	que->chunk_buff=malloc(que->total_size);
	que->queue_name=queue_name;
	memset(que->chunk_buff,0,que->total_size);
	return 1;
}
