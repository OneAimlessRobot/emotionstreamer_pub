#include "../../Includes/preprocessor.h"
#include <ncurses.h>
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



static int enqueue_chunk(chunk_queue* que,uint8_t* buff){

	if(!que_is_full(que)){
		memcpy(&que->chunk_buff[(que->recv_cursor)*que->chunk_size],buff,que->chunk_size);
		que->recv_cursor=(que->recv_cursor+1)%que->max_occupied;
		que->n_occupied++;
	}
	return que_is_full(que);


}
static int dequeue_chunk(chunk_queue* que,uint8_t* buff){

	if(!que_is_empty(que)){
		memcpy(buff,&que->chunk_buff[(que->play_cursor)*que->chunk_size],que->chunk_size);
		que->play_cursor=(que->play_cursor+1)%que->max_occupied;
		que->n_occupied--;
	}
	return que_is_empty(que);


}


static void clean_queue(chunk_queue* que){

		if(que->chunk_buff){
			free(que->chunk_buff);
		}
}

//ASSUMES INITSCREEN 
static void circular_q_visual_print(chunk_queue* que){

	if(!que){

		printw("Queue NULL\n");
		return;
	}
	if(!(que->chunk_buff)){

		printw("Chunk buff NULL na queue\n");
		return;
	}
	if(!(que->total_size)){

		
		printw("Queue com sizes null!\nSize total da queue: %u bytes\n"
							"Size de chunk da queue: %u bytes\n"
							"Numero de chunks totais da queue: %u bytes\n",
							que->total_size,
							que->chunk_size,
							que->max_occupied);
		return;
	}
	char* bar=malloc(PRINT_SIZE+3);
	memset(bar,0,PRINT_SIZE+3);
	bar[0]='[';
	memset(bar+1,' ',PRINT_SIZE);
	bar[PRINT_SIZE+1]=']';
	int play_cursor_bar_pos=1+(que->play_cursor*PRINT_SIZE)/que->max_occupied;
	int recv_cursor_bar_pos=1+(que->recv_cursor*PRINT_SIZE)/que->max_occupied;
	bar[play_cursor_bar_pos]='P';
	bar[recv_cursor_bar_pos]='R';
	
	int buff_ms=getQueueBufferedTime(que);
	for(int i=circular_int_inc(PRINT_SIZE+1,play_cursor_bar_pos);(play_cursor_bar_pos!=recv_cursor_bar_pos)&&(i!=recv_cursor_bar_pos);i=circular_int_inc(PRINT_SIZE+1,i)){

		bar[i]='=';
	}
	
	printw("Queue visual:\nplay cursor: %u\nrecv cursor: %u\nCurr occupied: %u\nMax occupied: %u\n",que->play_cursor,que->recv_cursor,que->n_occupied,que->max_occupied);
	printw("Queue esta quase vazia? %s\nQueue esta quase cheia? %s\nQueue esta vazia? %s\nQueue esta cheia? %s\nEstamos no byte %u\nTemos %u ms de audio no buffer!\n",
					que_is_almost_empty(que) ? "SIM":"NAO",
					que_is_almost_full(que)? "SIM":"NAO",
					que_is_empty(que) ? "SIM":"NAO",
					que_is_full(que)? "SIM":"NAO",que->n_occupied*que->chunk_size,buff_ms);
	printw("O buff:\n%s\n",bar);
	free(bar);
}


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
int perform_queue_op(chunk_queue* que,uint8_t* buff_if_insert,q_op op){
	int result=0;
	pthread_mutex_lock(que->queue_mtx);
	switch(op.main){

		case Q_LOOK:
			result=look_op(que,op.look);
			break;
		case Q_GET_TIME:
			result=(int)getQueueBufferedTime(que);
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
