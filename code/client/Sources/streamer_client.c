#include "../Includes/preprocessor.h"
#include <ncurses.h>
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_menus.h"
#include "../Includes/chunk_player.h"
#include "../Includes/streamer_client.h"


static int ncurses=1;
static pthread_cond_t cond1=PTHREAD_COND_INITIALIZER,
	       cond2=PTHREAD_COND_INITIALIZER,
	       cond3=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t mtx1=PTHREAD_MUTEX_INITIALIZER,
		mtx2=PTHREAD_MUTEX_INITIALIZER,
		mtx3=PTHREAD_MUTEX_INITIALIZER,
		mtx4=PTHREAD_MUTEX_INITIALIZER;

static pthread_t tid_rx,
	  tid_play,
	  tid_ack,
	  tid_stats;


static int lost_packet=0,
		reading=0,
		playing=0;

static client_stream_t stream_struct={
					0,
					0,
					NULL,
					NULL,
                                        NULL
                                        };


static void stop_client_stream(int useless){

	if(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		acess_var_mtx(&mtx4,&stream_struct.innited,0,V_SET);
       		pthread_cond_signal(&cond3+(0*useless));
		pthread_cond_signal(&cond2);
		pthread_cond_signal(&cond1);
	}

}
static void sigint_handler(int useless){

	printf("SIGINT! ");
	stop_client_stream((0*useless));

}

static void sigpipe_handler(int useless){

	printf("SIGPIPE! ");
	sigint_handler(useless);

}

static int read_chunk_tcp(client_stream_t* strm,int_pair pair){

	int result= readsome(strm->con_obj->sockfd_tcp,(char*)(strm->player->r_chunk),strm->player->chunk_size,pair);
	if(result<0){
		acess_var_mtx(&mtx4,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&mtx4,&lost_packet,0,V_SET);
	perform_queue_op(strm->que,strm->player->r_chunk,(q_op){Q_READ_FROM,Q_LOOK_NA});
	return result;
}
static int read_chunk_udp(client_stream_t* strm,int_pair pair){
        int result= readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->player->r_chunk),strm->player->chunk_size,pair,&strm->con_obj->peer_udp_addr);
	if(result<0){
		acess_var_mtx(&mtx4,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&mtx4,&lost_packet,0,V_SET);
	perform_queue_op(strm->que,strm->player->r_chunk,(q_op){Q_READ_FROM,Q_LOOK_NA});
	return result;
}


void* rx_thread_func(void* args){
	int full=0;
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		while((acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK))){

			read_chunk_udp(&stream_struct,client_data_times_pair);
			full=perform_queue_op(stream_struct.que,NULL,(q_op){Q_LOOK,Q_IS_FULL});
			pthread_cond_signal(&cond2);
			if(full){
				break;
			}
			acess_var_mtx(&mtx4,&reading,1,V_SET);
			con_send_udp(stream_struct.con_obj,client_data_times_pair);
		}
		pthread_mutex_lock(&mtx1);
		while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)&&perform_queue_op(stream_struct.que,NULL,(q_op){Q_LOOK,Q_IS_FULL})){

			acess_var_mtx(&mtx4,&reading,0,V_SET);
			pthread_cond_wait(&cond1,&mtx1);
		}
		pthread_mutex_unlock(&mtx1);
	
	}
	return args;
}
void* play_thread_func(void* args){

	int empty=0;
	usleep(cfg_latency_ms*1000);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		
			acess_var_mtx(&mtx4,&playing,1,V_SET);
			perform_queue_op(stream_struct.que,stream_struct.player->p_chunk,(q_op){Q_READ_TO,Q_LOOK_NA});
			perform_play_op(stream_struct.player,NULL,P_REAL_PLAY);
			empty=perform_queue_op(stream_struct.que,NULL,(q_op){Q_LOOK,Q_IS_EMPTY});
			pthread_cond_signal(&cond1);
			if(empty){
				break;
			}
	}
	pthread_mutex_lock(&mtx2);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)&&perform_queue_op(stream_struct.que,NULL,(q_op){Q_LOOK,Q_IS_EMPTY})){

		acess_var_mtx(&mtx4,&playing,0,V_SET);
		pthread_cond_wait(&cond2,&mtx2);
	}
	pthread_mutex_unlock(&mtx2);
	}
	return  args;
}

static void* ack_exchange_thread(void* args){

	int result=0;
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
        result=con_send_udp_ack(stream_struct.con_obj,client_data_times_pair);
        if(result<=0){
		if(result==-2){
                        stream_struct.curr_timeout++;
                        printf("Timeout no client!!!!  timeout %hu de %hu\n",stream_struct.curr_timeout,cfg_client_ack_timeout_lim);
                        if(stream_struct.curr_timeout==cfg_client_ack_timeout_lim){
                                break;
                        }
			continue;
	        }
                perror("");
        }
        result= con_read_udp_ack(stream_struct.con_obj,client_data_times_pair);
        if(result<=0){

                if(result==-2){
                        stream_struct.curr_timeout++;
                        printf("Timeout no client!!!!  timeout %hu de %hu\n",stream_struct.curr_timeout,cfg_client_ack_timeout_lim);
                        if(stream_struct.curr_timeout==cfg_client_ack_timeout_lim){
                                break;
                        }
        }
	}
        }

        raise(SIGINT);
        return args;


}

void* show_stats(void* args){

	if(ncurses){
        initscr();
	}
	//nodelay(stdscr,1);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		int time_ms=perform_queue_op(stream_struct.que,NULL,(q_op){Q_GET_TIME,Q_LOOK_NA});
		int pct_full=perform_queue_op(stream_struct.que,NULL,(q_op){Q_LOOK,Q_GET_PCT});
		usleep(10000);
		if(ncurses){
			erase();
			printw("Tempo restante no buffer, atualmente: %d ms\nPercentagem: %d\nReading?: %s Playing?: %s\n",
						time_ms,
						pct_full,
						acess_var_mtx(&mtx4,&reading,1,V_LOOK) ? "READ": "    ",
						acess_var_mtx(&mtx4,&playing,1,V_LOOK) ? "PLAY": "    ");

			perform_queue_op(stream_struct.que,NULL,(q_op){Q_PRINT,Q_LOOK_NA});
			refresh();
		}
		else{
			system("clear");
			printf("Tempo restante no buffer, atualmente: %d ms\nPercentagem: %d\nReading?: %s Playing?: %s\n",
						time_ms,
						pct_full,
						acess_var_mtx(&mtx4,&reading,1,V_LOOK) ? "READ": "    ",
						acess_var_mtx(&mtx4,&playing,1,V_LOOK) ? "PLAY": "    ");


		}
	}
	if(ncurses){
		endwin();
	}
	return args;
}


static int init_client_stream(con_t* con_obj, uint16_t chunk_size,method which_mode){
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	
	uint8_t r_chunk_buff[chunk_size];
	memset(r_chunk_buff,0,chunk_size);
	uint8_t p_chunk_buff[chunk_size];
	memset(p_chunk_buff,0,chunk_size);
	chunk_queue que={0};
	chunk_player player={0};
	init_queue(&que,chunk_size);
	init_chunk_player(&player,chunk_size,p_chunk_buff,r_chunk_buff,which_mode);
	stream_struct.que=&que;
	stream_struct.player=&player;
	
	stream_struct.con_obj=con_obj;
	stream_struct.innited=1;
	
	pthread_create(&tid_ack,NULL,ack_exchange_thread,NULL);
	pthread_create(&tid_rx,NULL,rx_thread_func,NULL);
	pthread_create(&tid_play,NULL,play_thread_func,NULL);
	pthread_create(&tid_stats,NULL,show_stats,NULL);
	
	pthread_mutex_lock(&mtx3);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		pthread_cond_wait(&cond3,&mtx3);
	}
	pthread_mutex_unlock(&mtx3);
	pthread_join(tid_stats,NULL);
	printf("Saimos do thread stats!!!!!!\n");
	pthread_join(tid_play,NULL);
	printf("Saimos do thread play!!!!!!\n");
	pthread_join(tid_rx,NULL);
	printf("Saimos do thread rx!!!!!\n");
	pthread_join(tid_ack,NULL);
	printf("Saimos do thread ack!!!!!\n");
	
	perform_queue_op(stream_struct.que,NULL,(q_op){Q_CLEAN,Q_LOOK_NA});
	perform_play_op(stream_struct.player,NULL,P_CLEAN);
	close_con(stream_struct.con_obj);
	printf("SAIMOS DO CLIENT!\nTimeouts excedidos? %s\nVamos ver errno:%s\n",(stream_struct.curr_timeout==cfg_client_ack_timeout_lim) ? "SIM": "NAO",strerror(errno));
	return 0;
}



void player_stop_stream(void){
	raise(SIGINT);
}

void player_init_stream(con_t* con_obj,uint16_t chunk_size,method which_mode){

	init_client_stream(con_obj,chunk_size,which_mode);
}

