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

#include "../../minimp3/minimp3.h"
#include "../Includes/mp3module.h"

#include "../Includes/chunk_player.h"

#include "../Includes/streamer_client.h"

static int ncurses=1;
static int stats=1;
static int play=1;
static int decode=1;
static int rx_enabled=1;

static pthread_cond_t cond1=PTHREAD_COND_INITIALIZER,
	       cond2=PTHREAD_COND_INITIALIZER,
	       cond3=PTHREAD_COND_INITIALIZER,
	       cond4=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t mtx1=PTHREAD_MUTEX_INITIALIZER,
		mtx2=PTHREAD_MUTEX_INITIALIZER,
		mtx3=PTHREAD_MUTEX_INITIALIZER,
		mtx4=PTHREAD_MUTEX_INITIALIZER,
		mtx5=PTHREAD_MUTEX_INITIALIZER;

static pthread_t tid_rx,
	  tid_play,
	  tid_dec,
	  tid_ack,
	  tid_stats;


static int lost_packet=0,
		reading=0,
		decoding=0,
		playing=0;

static client_stream_t stream_struct={
					0,
					0,
					NULL,
					NULL,
                                        NULL,
                                        NULL,
					NULL,
                                        NULL
                                        };


static void stop_client_stream(int useless){

	if(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		acess_var_mtx(&mtx4,&stream_struct.innited,0,V_SET);
       		pthread_cond_signal(&cond3+(0*useless));
		pthread_cond_signal(&cond2);
		pthread_cond_signal(&cond4);
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

	int result= readsome(strm->con_obj->sockfd_tcp,(char*)(strm->decoder->r_chunk),strm->decoder->d_chunk_size,pair);
	if(result<0){
		acess_var_mtx(&mtx4,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&mtx4,&lost_packet,0,V_SET);
	perform_queue_op(strm->decoder_que,strm->decoder->r_chunk,0,(q_op){Q_READ_FROM,Q_LOOK_NA},NULL);
	return result;
}
static int read_chunk_udp(client_stream_t* strm,int_pair pair){
        int result= readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->decoder->r_chunk),strm->decoder->d_chunk_size,pair,&strm->con_obj->peer_udp_addr);
	if(result<0){
		acess_var_mtx(&mtx4,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&mtx4,&lost_packet,0,V_SET);
	perform_queue_op(strm->decoder_que,strm->decoder->r_chunk,0,(q_op){Q_READ_FROM,Q_LOOK_NA},NULL);
	return result;
}


void* rx_thread_func(void* args){
	int full=0;
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		while((acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK))){

			read_chunk_tcp(&stream_struct,client_data_times_pair);
			full=perform_queue_op(stream_struct.decoder_que,NULL,0,(q_op){Q_LOOK,Q_IS_FULL},NULL);
			acess_var_mtx(&mtx4,&reading,1,V_SET);
			if(full){
				pthread_cond_signal(&cond4);
				break;
			}
			con_send_udp(stream_struct.con_obj,client_data_times_pair);
		}
		pthread_mutex_lock(&mtx1);
		while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)&&perform_queue_op(stream_struct.decoder_que,NULL,0,(q_op){Q_LOOK,Q_IS_FULL},NULL)){

			acess_var_mtx(&mtx4,&reading,0,V_SET);
			pthread_cond_wait(&cond1,&mtx1);
		}
		pthread_mutex_unlock(&mtx1);
	
	}
	return args;
}
void* dec_thread_func(void* args){

	int empty=0;
	int full=0;
	mp3decoder_result_struct result={0};
	pthread_mutex_lock(&mtx5);
	while(!acess_var_mtx(&mtx4,&reading,0,V_LOOK)&&acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		pthread_cond_wait(&cond4,&mtx5);
	}
	pthread_mutex_unlock(&mtx5);
	if(!ncurses){
		printf("Thread de decoding alcançado!\n");
	}
	usleep(1000*cfg_latency_ms);
	pthread_cond_signal(&cond2);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
				
			while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
				perform_queue_op(stream_struct.decoder_que,stream_struct.decoder->d_chunk,0,(q_op){Q_READ_TO,Q_LOOK_NA},NULL);
				while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
					perform_dec_op(stream_struct.decoder,&result,D_DECODE_CHUNK_SIMPLE);
					if(perform_dec_op(stream_struct.decoder,NULL,D_IS_D_BUFFER_EMPTY)||result.frame_bytes){
						
						if(result.nsamples){
							perform_queue_op(stream_struct.player_que,stream_struct.decoder->p_chunk,0,(q_op){Q_READ_FROM,Q_LOOK_NA},NULL);
							//perform_queue_op(stream_struct.auxiliar_que,(uint8_t*)&result,0,(q_op){Q_READ_FROM,Q_LOOK_NA},NULL);
							//print_decoder_frame_result(&result,1);
						}
						else{

							
							perform_dec_op(stream_struct.decoder,NULL,D_RESET_MP3_PTR);
							break;

						}
					}
				}
				if(perform_dec_op(stream_struct.decoder,NULL,D_IS_P_BUFFER_FULL)){
					perform_dec_op(stream_struct.decoder,NULL,D_RESET_PCM_PTR);
					break;

				}

			}
			full=perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_LOOK,Q_IS_FULL},NULL);
			acess_var_mtx(&mtx4,&decoding,1,V_SET);
			pthread_cond_signal(&cond1);
			pthread_cond_signal(&cond2);
			if(full){

				break;
			}
	}
	pthread_mutex_lock(&mtx5);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		acess_var_mtx(&mtx4,&decoding,0,V_SET);
		pthread_cond_wait(&cond4,&mtx5);
	}
	pthread_mutex_unlock(&mtx5);
	}
	return  args;
}

void* play_thread_func(void* args){

	int empty=0;
	mp3decoder_result_struct result={0};
	pthread_mutex_lock(&mtx2);
	while(!acess_var_mtx(&mtx4,&decoding,0,V_LOOK)&&acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		pthread_cond_wait(&cond2,&mtx2);
	}
	pthread_mutex_unlock(&mtx2);
	if(!ncurses){

		printf("Thread de play alcançado!\n");
	}
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){
		
			acess_var_mtx(&mtx4,&playing,1,V_SET);
			perform_queue_op(stream_struct.player_que,stream_struct.player->p_chunk,0,(q_op){Q_READ_TO,Q_LOOK_NA},NULL);
			//perform_queue_op(stream_struct.auxiliar_que,(uint8_t*)&result,0,(q_op){Q_READ_TO,Q_LOOK_NA},NULL);
			perform_play_op(stream_struct.player,&result,P_REAL_PLAY);
			empty=perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_LOOK,Q_IS_EMPTY},NULL);
			if(empty){
				pthread_cond_signal(&cond4);
				break;
			}
	}
	pthread_mutex_lock(&mtx2);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)&&perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_LOOK,Q_IS_EMPTY},NULL)){
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
		int time_ms=perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_GET_TIME,Q_LOOK_NA},NULL);
		int pct_full_playing=perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_LOOK,Q_GET_PCT},NULL);
		int pct_full_decoding=perform_queue_op(stream_struct.decoder_que,NULL,0,(q_op){Q_LOOK,Q_GET_PCT},NULL);
		usleep(10000);
		if(ncurses){
			erase();
			printw("Tempo restante no buffer, atualmente: %d ms\nPercentagem de preenchimento em playing: %d\nPercentagem de preenchimento em decoding: %d\nReading?: %s Decoding?: %s Playing?: %s\n",
						time_ms,
						pct_full_playing,
						pct_full_decoding,
						acess_var_mtx(&mtx4,&reading,1,V_LOOK) ? "READ": "    ",
						acess_var_mtx(&mtx4,&decoding,1,V_LOOK) ? "DECODING": "    ",
						acess_var_mtx(&mtx4,&playing,1,V_LOOK) ? "PLAY": "    ");

			perform_queue_op(stream_struct.decoder_que,NULL,0,(q_op){Q_PRINT,Q_LOOK_NA},NULL);
			perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_PRINT,Q_LOOK_NA},NULL);
			refresh();
		}
		else{
			system("clear");
			printf("Tempo restante no buffer, atualmente: %d ms\nPercentagem de preenchimento em playing: %d\nPercentagem de preenchimento em decoding: %d\nReading?: %s %s Decoding?: Playing?: %s\n",
						time_ms,
						pct_full_playing,
						pct_full_decoding,
						acess_var_mtx(&mtx4,&reading,1,V_LOOK) ? "READ": "    ",
						acess_var_mtx(&mtx4,&decoding,1,V_LOOK) ? "DECODING": "    ",
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
	uint8_t d_chunk_buff[chunk_size];
	memset(d_chunk_buff,0,chunk_size);
	uint8_t pd_chunk_buff[MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE];
	memset(pd_chunk_buff,0,MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE);
	uint8_t pp_chunk_buff[MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE];
	memset(pp_chunk_buff,0,MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE);
	chunk_queue player_que={0};
	chunk_queue decoder_que={0};
	chunk_queue auxiliar_que={0};
	chunk_player player={0};
	mp3decoder decoder={0};
	init_queue(&player_que,MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE);
	init_queue(&decoder_que,chunk_size);
	init_queue(&auxiliar_que,sizeof(mp3decoder_result_struct));

	init_mp3_decoder(&decoder,chunk_size,MINIMP3_MAX_SAMPLES_PER_FRAME,r_chunk_buff,d_chunk_buff,pd_chunk_buff);
	init_chunk_player(&player,MINIMP3_MAX_SAMPLES_PER_FRAME*SIZE,pp_chunk_buff,which_mode);
	stream_struct.decoder_que=&decoder_que;
	stream_struct.player_que=&player_que;
	stream_struct.auxiliar_que=&auxiliar_que;
	stream_struct.player=&player;
	stream_struct.decoder=&decoder;
	
	stream_struct.con_obj=con_obj;
	stream_struct.innited=1;
	
	pthread_create(&tid_ack,NULL,ack_exchange_thread,NULL);
	if(rx_enabled){
		pthread_create(&tid_rx,NULL,rx_thread_func,NULL);
	}
	if(decode){
		pthread_create(&tid_dec,NULL,dec_thread_func,NULL);
	}
	if(play){
		pthread_create(&tid_play,NULL,play_thread_func,NULL);
	}
	if(stats){
		pthread_create(&tid_stats,NULL,show_stats,NULL);
	}
	
	pthread_mutex_lock(&mtx3);
	while(acess_var_mtx(&mtx4,&stream_struct.innited,0,V_LOOK)){

		pthread_cond_wait(&cond3,&mtx3);
	}
	pthread_mutex_unlock(&mtx3);
	if(stats){
		pthread_join(tid_stats,NULL);
		printf("Saimos do thread stats!!!!!!\n");
	}
	if(play){
		pthread_join(tid_play,NULL);
		printf("Saimos do thread play!!!!!!\n");
	}
	if(decode){
	
		pthread_join(tid_dec,NULL);
		printf("Saimos do thread decoder!!!!!\n");
	}
	if(rx_enabled){
		pthread_join(tid_rx,NULL);
		printf("Saimos do thread rx!!!!!\n");
	}
	pthread_join(tid_ack,NULL);
	printf("Saimos do thread ack!!!!!\n");
	
	perform_queue_op(stream_struct.player_que,NULL,0,(q_op){Q_CLEAN,Q_LOOK_NA},NULL);
	perform_queue_op(stream_struct.decoder_que,NULL,0,(q_op){Q_CLEAN,Q_LOOK_NA},NULL);
	perform_queue_op(stream_struct.auxiliar_que,NULL,0,(q_op){Q_CLEAN,Q_LOOK_NA},NULL);
	perform_play_op(stream_struct.player,NULL,P_CLEAN);
	perform_dec_op(stream_struct.decoder,NULL,D_CLEAN);
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

