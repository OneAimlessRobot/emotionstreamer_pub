#include "../../Includes/preprocessor.h"
#include <ncurses.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_arithmetic.h"
#include "../Includes/queue_menus.h"
#include "../Includes/terminal_mgmt.h"

#define BUFFSIZE 2048
static int dequeue_chunk(chunk_queue* que,uint8_t* buff){

	if(!que_is_empty(que)){
		memcpy(buff,&que->chunk_buff[(que->play_cursor)*que->chunk_size],que->chunk_size);
		que->play_cursor=(que->play_cursor+1)%que->max_occupied;
		que->n_occupied--;
	}
	return que_is_empty(que);


}

static int enqueue_chunk(chunk_queue* que,uint8_t* buff){

	if(!que_is_full(que)){
		memcpy(&que->chunk_buff[(que->recv_cursor)*que->chunk_size],buff,que->chunk_size);
		que->recv_cursor=(que->recv_cursor+1)%que->max_occupied;
		que->n_occupied++;
	}
	return que_is_full(que);


}

static void clean_queue(chunk_queue* que){

		if(que->chunk_buff){
			free(que->chunk_buff);
		}
}

//ASSUMES INITSCREEN 


static int look_op(chunk_queue* que,queue_look_op op){
	int result=-1;
	switch(op){
		case Q_IS_FULL:
			result=que_is_full(que);
			break;
		case Q_IS_EMPTY:
			result=que_is_empty(que);
			break;
		case Q_IS_ALMOST_EMPTY:
			result=que_is_almost_empty(que);
			break;
		case Q_IS_ALMOST_FULL:
			result=que_is_almost_full(que);
			break;
		case Q_GET_PCT:
			result=que_pct_full(que);
			break;
		case Q_LOOK_NA:
			break;
		default:
			break;
	}

	return result;

}
static void circular_q_visual_print(chunk_queue* que){

	if(!que){

		print_string("Queue NULL\n");
		return;
	}
	if(!(que->chunk_buff)){

		print_string("Queue buff NULL na queue\n");
		return;
	}
	if(!(que->total_size)){
		char buff[BUFFSIZE]={0};
		snprintf(buff,BUFFSIZE-1,"Queue com sizes null!\nSize total da queue: %lu bytes\n"
							"Size de chunk da queue: %lu bytes\n"
							"Numero de chunks totais da queue: %lu bytes\n",
							que->total_size,
							que->chunk_size,
							que->max_occupied);
		print_string(buff);
		return;
	}
	char bar[que->display_size+3];
	memset(bar,0,que->display_size+3);
	bar[0]='[';
	memset(bar+1,' ',que->display_size);
	bar[que->display_size+1]=']';
	uint64_t play_cursor_bar_pos=1+(que->play_cursor*que->display_size)/que->max_occupied;
	uint64_t recv_cursor_bar_pos=1+(que->recv_cursor*que->display_size)/que->max_occupied;
	bar[play_cursor_bar_pos]='P';
	bar[recv_cursor_bar_pos]='R';
	for(uint64_t i=circular_int_inc(que->display_size+1,play_cursor_bar_pos);(play_cursor_bar_pos!=recv_cursor_bar_pos)&&(i!=recv_cursor_bar_pos);i=circular_int_inc(que->display_size+1,i)){

		if(i&&(i<que->display_size)){
			bar[i]='=';
		}
	}
	char buff[BUFFSIZE]={0};
	int inc=snprintf(buff,BUFFSIZE-1,"Queue name: %s\n",que->queue_name);
	snprintf(buff+inc,BUFFSIZE-1,"%s\n",bar);
	print_string(buff);
}
int perform_queue_op(chunk_queue* que,uint8_t* buff_if_insert, decoder_result_struct* frame_data_struct,q_op op){
	int result=0;
	pthread_mutex_lock(que->queue_mtx);
	switch(op.main){

		case Q_LOOK:
			result=look_op(que,op.look);
			break;
		case Q_GET_TIME:
			result=(int)getQueueBufferedTime(que,frame_data_struct);
			break;
		case Q_PRINT:
			circular_q_visual_print(que);
			break;
		case Q_READ_TO:
			result=dequeue_chunk(que,buff_if_insert);
			break;
		case Q_READ_FROM:
			result=enqueue_chunk(que,buff_if_insert);
			break;
		case Q_CLEAN:
			clean_queue(que);
			break;
		case Q_NA:
			break;
	}
	pthread_mutex_unlock(que->queue_mtx);
	return result;

}
