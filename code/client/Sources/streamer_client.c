#include "../Includes/preprocessor.h"
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_menus.h"

#include "../Includes/mp3module.h"

#include "../Includes/chunk_player.h"

#include "../Includes/streamer_client.h"
#include "../Includes/terminal_mgmt.h"

static const int play=1;
static const int decode=1;
static const int rx_enabled=1;
static const int input_enabled=1;
static struct sigaction sa;

atomic_int innited=0;

static pthread_cond_t reading_cond=PTHREAD_COND_INITIALIZER,
	       player_cond=PTHREAD_COND_INITIALIZER,
	       running_cond=PTHREAD_COND_INITIALIZER,
	       input_cond=PTHREAD_COND_INITIALIZER,
	       decoder_cond=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t reading_mtx=PTHREAD_MUTEX_INITIALIZER,
		player_mtx=PTHREAD_MUTEX_INITIALIZER,
		running_mtx=PTHREAD_MUTEX_INITIALIZER,
		variable_acess_mtx=PTHREAD_MUTEX_INITIALIZER,
		input_mtx=PTHREAD_MUTEX_INITIALIZER,
		ncurses_mtx=PTHREAD_MUTEX_INITIALIZER,
		decoder_mtx=PTHREAD_MUTEX_INITIALIZER;


static pthread_t tid_rx,
	  tid_play,
	  tid_dec,
	  tid_ack,
	  tid_input,
	  tid_stats;


static int lost_packet=0,
		reading=0,
		decoding=0,
		playing=0,
		paused=0;
static int is_first_player_chunk=1;

static client_stream_t stream_struct={
					0,
					NULL,
					NULL,
                                        NULL,
					NULL,
                                        NULL
                                        };

static void endwin_wrapper(void){

	pthread_mutex_lock(&ncurses_mtx);
	if(is_raw(STDOUT)){

		print_string("Chamamos disable raw em out!!!!\n");
		disable_raw(STDOUT);
	}
	if(is_raw(STDIN)){

		print_string("Chamamos disable raw em in!!!!\n");
		disable_raw(STDIN);
	}
	pthread_mutex_unlock(&ncurses_mtx);

}

static void stop_client_stream(void){

	pthread_cond_signal(&running_cond);
	pthread_cond_signal(&player_cond);
	pthread_cond_signal(&decoder_cond);
	pthread_cond_signal(&reading_cond);
	pthread_cond_signal(&input_cond);
	endwin_wrapper();
        if(acess_var_mtx(&variable_acess_mtx,&stream_struct.con_obj->is_on,0,V_LOOK)){
		send_port_back(htons(stream_struct.con_obj->this_tcp_addr.sin_port),&client_port_mapper_ip_cache_entry);
		send_ports_back(stream_struct.con_obj);
		close_con(stream_struct.con_obj);
	}


}
static void sigint_handler(int useless){

	innited=0*useless;
}

static int read_chunk_tcp(client_stream_t* strm,int_pair pair){

	int result=-1;
	uint8_t chk[sizeof(mp3_stream_chunk)]={0};
	if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
		result= readsome(strm->con_obj->sockfd_tcp,(char*)(strm->player->h_chunk),strm->player->chunk_size,pair);
	}
	else{
		result= readsome(strm->con_obj->sockfd_tcp,(char*)((is_wav_mode||!decode)?strm->player->r_chunk:chk),(is_wav_mode||!decode)?strm->player->chunk_size:sizeof(frame_info_t)+4+strm->decoder->d_chunk_size,pair);
		if(decode&&!is_wav_mode){
			memcpy(strm->decoder->r_chunk,&chk,sizeof(frame_info_t)+4+strm->decoder->d_chunk_size);
		}
	}
	if(result<0){
		acess_var_mtx(&variable_acess_mtx,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&variable_acess_mtx,&lost_packet,0,V_SET);
	if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
		perform_play_op(stream_struct.player,NULL,P_INIT_LIBS);
		acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_SET);
	}
	else{
		perform_queue_op((is_wav_mode||!decode)?strm->player_que:strm->decoder_que,(is_wav_mode||!decode)?strm->player->r_chunk:strm->decoder->r_chunk,NULL,(q_op){Q_READ_FROM,Q_LOOK_NA});
	}
	return result;
}
static int read_chunk_udp(client_stream_t* strm,int_pair pair){
        int result=-1;
	uint8_t chk[sizeof(mp3_stream_chunk)]={0};
	if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
		result= readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->player->h_chunk),strm->player->chunk_size,pair,&strm->con_obj->peer_udp_addr);
	}
	else{
		result= readsome_udp(strm->con_obj->sockfd_udp,(char*)((is_wav_mode||!decode)?strm->player->r_chunk:chk),(is_wav_mode||!decode)?strm->player->chunk_size:sizeof(frame_info_t)+4+strm->decoder->d_chunk_size,pair,&strm->con_obj->peer_udp_addr);
		if(decode&&!is_wav_mode){
			memcpy(strm->decoder->r_chunk,&chk,sizeof(frame_info_t)+4+strm->decoder->d_chunk_size);
		}
	}
	if(result<0){
		acess_var_mtx(&variable_acess_mtx,&lost_packet,1,V_SET);
		return result;
	}
	acess_var_mtx(&variable_acess_mtx,&lost_packet,0,V_SET);
	if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
		perform_play_op(stream_struct.player,NULL,P_INIT_LIBS);
		acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_SET);
	}
	else{
		perform_queue_op((is_wav_mode||!decode)?strm->player_que:strm->decoder_que,(is_wav_mode||!decode)?strm->player->r_chunk:strm->decoder->r_chunk,NULL,(q_op){Q_READ_FROM,Q_LOOK_NA});
	}
	return result;
}


static void* rx_thread_func(void* args){
	int full=0;
	print_string("Thread de reading alcançado!\n");
	while(innited){
		acess_var_mtx(&variable_acess_mtx,&reading,1,V_SET);
		while(innited){
			(streaming_protocol<=0)?read_chunk_tcp(&stream_struct,client_data_times_pair):read_chunk_udp(&stream_struct,client_data_times_pair);
			if(decode&&!is_wav_mode){
				pthread_cond_signal(&decoder_cond);
			}
			else if(!acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
				pthread_cond_signal(&player_cond);
			}
			if(input_enabled){
				pthread_cond_signal(&input_cond);
			}
			con_send_udp(stream_struct.con_obj,client_data_times_pair);
			if(acess_var_mtx(&variable_acess_mtx,&paused,0,V_LOOK)){

				break;
			}
			full=perform_queue_op((is_wav_mode||!decode)?stream_struct.player_que:stream_struct.decoder_que,NULL,NULL,(is_wav_mode||!decode)?(q_op){Q_LOOK,Q_IS_FULL}:(q_op){Q_LOOK,Q_IS_FULL});
			if(full){
				break;
			}
			if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){
				break;
			}
		}
		pthread_mutex_lock(&reading_mtx);
		while(innited&&(acess_var_mtx(&variable_acess_mtx,&paused,0,V_LOOK)||(perform_queue_op((is_wav_mode||!decode)?stream_struct.player_que:stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,(is_wav_mode||!decode)?Q_IS_FULL:Q_IS_ALMOST_FULL})))){

			acess_var_mtx(&variable_acess_mtx,&reading,0,V_SET);
			pthread_cond_wait(&reading_cond,&reading_mtx);
		}
		pthread_mutex_unlock(&reading_mtx);
	}
	return args;
}

static void* dec_thread_func(void* args){
	int empty=0;
	int full=0;
	int ret_val=MPG123_NEED_MORE;
	char buff[1024]={0};
	pthread_mutex_lock(&decoder_mtx);
	while(!acess_var_mtx(&variable_acess_mtx,&reading,0,V_LOOK)&&innited){

		pthread_cond_wait(&decoder_cond,&decoder_mtx);
	}
	pthread_mutex_unlock(&decoder_mtx);
	print_string("Thread de decoding alcançado!\n");
	usleep(cfg_latency_ms*1000);
	while(innited){
		acess_var_mtx(&variable_acess_mtx,&decoding,1,V_SET);
		while(innited){
			perform_queue_op(stream_struct.decoder_que,stream_struct.decoder->d_chunk,NULL,(q_op){Q_READ_TO,Q_LOOK_NA});
			ret_val=perform_dec_op(stream_struct.decoder,D_DECODE_CHUNK,DO_DECODE);
			if(((mp3_processed_chunk*)stream_struct.decoder->p_chunk)->result_struct.decoder_state){
				perform_dec_op(stream_struct.decoder,D_RESET_BOTH,DO_DECODE);
				perform_queue_op(stream_struct.player_que,stream_struct.decoder->p_chunk,NULL,(q_op){Q_READ_FROM,Q_LOOK_NA});
				pthread_cond_signal(&player_cond);
				full=perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_FULL});
				if(full){
					break;
				}
			}
			pthread_cond_signal(&reading_cond);
			if(acess_var_mtx(&variable_acess_mtx,&paused,1,V_LOOK)){

				break;
			}
			empty=perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_EMPTY});
			if(empty){
				break;
			}
			if(!(ret_val==MPG123_NEED_MORE)){
				if(ret_val==MPG123_DONE){
					memset(buff,0,1024);
					snprintf(buff,1023,"Stream done!\n");
					print_string(buff);
					raise(SIGINT);
        				stop_client_stream();
        			}
				else if(ret_val==MPG123_ERR){
					memset(buff,0,1024);
					snprintf(buff,1023,"Decoding error: %s\n",mpg123_strerror(stream_struct.decoder->dec));
					print_string(buff);
					raise(SIGINT);
					stop_client_stream();
        			}
			}
	}
	pthread_mutex_lock(&decoder_mtx);
	while(innited&&(acess_var_mtx(&variable_acess_mtx,&paused,0,V_LOOK)||perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_FULL})||perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_ALMOST_EMPTY}))){

		acess_var_mtx(&variable_acess_mtx,&decoding,0,V_SET);
		pthread_cond_wait(&decoder_cond,&decoder_mtx);
	}
	pthread_mutex_unlock(&decoder_mtx);
	}

	return  args;
}

static void* play_thread_func(void* args){

	int empty=0;
	pthread_mutex_lock(&player_mtx);
	while((decode&&!is_wav_mode)?!acess_var_mtx(&variable_acess_mtx,&decoding,0,V_LOOK):!acess_var_mtx(&variable_acess_mtx,&reading,0,V_LOOK)&&innited){

		pthread_cond_wait(&player_cond,&player_mtx);
	}
	pthread_mutex_unlock(&player_mtx);
	print_string("Thread de play alcançado!\n");
	usleep(cfg_latency_ms*1000);
	while(innited){
		acess_var_mtx(&variable_acess_mtx,&playing,1,V_SET);
		while(innited){
			if(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK)){

				continue;
			}
			perform_queue_op(stream_struct.player_que,stream_struct.player->p_chunk,NULL,(q_op){Q_READ_TO,Q_LOOK_NA});
			perform_play_op(stream_struct.player,NULL,P_REAL_PLAY);
			if(acess_var_mtx(&variable_acess_mtx,&paused,1,V_LOOK)){

				break;
			}
			if(!is_wav_mode){
				pthread_cond_signal(&decoder_cond);
			}
			else{

				pthread_cond_signal(&reading_cond);
			}
			empty=perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_EMPTY});
			if(empty){
				break;
			}
	}
	pthread_mutex_lock(&player_mtx);
	while(innited&&(acess_var_mtx(&variable_acess_mtx,&paused,0,V_LOOK)||perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_ALMOST_EMPTY}))){
		acess_var_mtx(&variable_acess_mtx,&playing,0,V_SET);
		pthread_cond_wait(&player_cond,&player_mtx);
	}
	pthread_mutex_unlock(&player_mtx);
	}
	return  args;
}
static void* ack_exchange_thread(void* args){

	int result=0;
	char buff[1024]={0};
	while(innited){
        result=con_send_udp_ack(stream_struct.con_obj,client_data_times_pair);
        if(result<=0){
		if(result==-2){
                        stream_struct.curr_timeout++;
                        snprintf(buff,1023,"Timeout no client!!!!  timeout %lu de %lu\n",stream_struct.curr_timeout,cfg_client_ack_timeout_lim);
                        print_string(buff);
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
                        snprintf(buff,1023,"Timeout no client!!!!  timeout %lu de %lu\n",stream_struct.curr_timeout,cfg_client_ack_timeout_lim);
                        print_string(buff);
			if(stream_struct.curr_timeout==cfg_client_ack_timeout_lim){
                                break;
                        }
        }
	}
        }

	raise(SIGINT);
        stop_client_stream();
        return args;


}

static void* show_stats(void* args){

	if(stream_enable_ncurses){
        	enable_raw(STDOUT);
	}
	while(innited){
		usleep(1000);
		decoder_result_struct result={0};
		
		perform_play_op(stream_struct.player,&result,P_GET_FRAME_DATA);
		int pct_full_decoding=0;
		int time_ms=perform_queue_op(stream_struct.player_que,NULL,&result,(q_op){Q_GET_TIME,Q_LOOK_NA});
		int pct_full_playing=perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_GET_PCT});
		if(decode&&!is_wav_mode){
			pct_full_decoding=perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,Q_GET_PCT});
		}
		system("clear");
		char buff[1024]={0};
		snprintf(buff,1023,"Tempo restante no buffer, atualmente: %d ms\nPercentagem de preenchimento em playing: %d\nPercentagem de preenchimento em decoding: %d\nReading?: %sDecoding?: %s Playing?: %s Paused?: %s\nAre we yet to receive the WAV header? %s\n\n",
					time_ms,
					pct_full_playing,
					pct_full_decoding,
					acess_var_mtx(&variable_acess_mtx,&reading,1,V_LOOK) ? "READING ": "    ",
					acess_var_mtx(&variable_acess_mtx,&decoding,1,V_LOOK) ? "DECODING ": "    ",
					acess_var_mtx(&variable_acess_mtx,&playing,1,V_LOOK) ? "PLAYING ": "    ",
					acess_var_mtx(&variable_acess_mtx,&paused,1,V_LOOK) ? "PAUSED ": "    ",
					is_wav_mode?(acess_var_mtx(&variable_acess_mtx,&is_first_player_chunk,0,V_LOOK) ? "YES! ": "NO..."):"Not in wav mode...");
		print_string(buff);
		if(stream_enable_ncurses){
			if(decode&&!is_wav_mode){
				perform_queue_op(stream_struct.decoder_que,NULL,&result,(q_op){Q_PRINT,Q_LOOK_NA});
			}
			perform_queue_op(stream_struct.player_que,NULL,&result,(q_op){Q_PRINT,Q_LOOK_NA});
		}
	}
	endwin_wrapper();
	return args;
}

static void* input_thread_func(void* args){

	pthread_mutex_lock(&input_mtx);
        while((decode&&!is_wav_mode)?!acess_var_mtx(&variable_acess_mtx,&decoding,0,V_LOOK):!acess_var_mtx(&variable_acess_mtx,&reading,0,V_LOOK)&&innited){

                pthread_cond_wait(&input_cond,&input_mtx);
        }
	pthread_mutex_unlock(&input_mtx);

	while(innited){

		char input_buff[DEF_DATASIZE+1]={0};
		scanf("%s",input_buff);
		switch(input_buff[0]){

			case 'p':
				pthread_mutex_lock(&input_mtx);
				int pause_value=acess_var_mtx(&variable_acess_mtx,&paused,0,V_LOOK);
				if(pause_value){

					pthread_cond_signal(&reading_cond);
					pthread_cond_signal(&player_cond);
				}
				acess_var_mtx(&variable_acess_mtx,&paused,!pause_value,V_SET);
				pthread_mutex_unlock(&input_mtx);
			break;
			default:
			break;
		}


	}
	endwin_wrapper();
	return args;

}
static int init_client_stream(con_t* con_obj, uint16_t chunk_size,method which_mode){
        sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGPIPE, &sa, NULL);
	
	
	uint8_t h_chunk_buff[is_wav_mode?chunk_size:1];
	memset(h_chunk_buff,0,sizeof(h_chunk_buff));
	uint8_t r_chunk_buff[sizeof(frame_info_t)+4+chunk_size];
	memset(r_chunk_buff,0,sizeof(r_chunk_buff));
	uint8_t d_chunk_buff[(!decode||is_wav_mode)?1:sizeof(frame_info_t)+4+chunk_size];
	memset(d_chunk_buff,0,sizeof(d_chunk_buff));
	uint8_t pd_chunk_buff[(!decode||is_wav_mode)?1:sizeof(decoder_result_struct)+4+chunk_size+1];
	memset(pd_chunk_buff,0,sizeof(pd_chunk_buff));
	uint8_t pp_chunk_buff[(!decode||is_wav_mode)?chunk_size:sizeof(decoder_result_struct)+4+chunk_size+1];
	memset(pp_chunk_buff,0,sizeof(pd_chunk_buff));
	chunk_queue player_que={0};
	chunk_queue decoder_que={0};
	chunk_player player={0};
	decoder_t decoder={0};
	init_queue(&player_que,(!decode||is_wav_mode)?chunk_size:sizeof(pd_chunk_buff),cfg_stream_player_cache_size_chunks);
	init_chunk_player(&player,(!decode||is_wav_mode)?chunk_size:sizeof(pp_chunk_buff),h_chunk_buff,r_chunk_buff,pp_chunk_buff,which_mode);
	stream_struct.player_que=&player_que;
	stream_struct.player=&player;
	if(decode&&!is_wav_mode){
		printf("Decoder will be initialized\n");
		init_queue(&decoder_que,sizeof(frame_info_t)+4+chunk_size,cfg_stream_decoder_cache_size_chunks);
		init_decoder(&decoder,sizeof(frame_info_t)+4+chunk_size,sizeof(decoder_result_struct)+4+chunk_size,r_chunk_buff,d_chunk_buff,pd_chunk_buff/*,h2_chunk_buff*/);
		stream_struct.decoder=&decoder;
		stream_struct.decoder_que=&decoder_que;
		is_first_player_chunk=0;
	}
	
	stream_struct.con_obj=con_obj;
	innited=1;
	pthread_create(&tid_ack,NULL,ack_exchange_thread,NULL);
	if(rx_enabled){
		pthread_create(&tid_rx,NULL,rx_thread_func,NULL);
	}
	if(decode&&!is_wav_mode){
		pthread_create(&tid_dec,NULL,dec_thread_func,NULL);
	}
	if(play){
		pthread_create(&tid_play,NULL,play_thread_func,NULL);
	}
	if(stream_show_stats){
		pthread_create(&tid_stats,NULL,show_stats,NULL);
	}
	if(input_enabled){
		pthread_create(&tid_input,NULL,input_thread_func,NULL);
	}
	
	pthread_mutex_lock(&running_mtx);
	while(innited){

		pthread_cond_wait(&running_cond,&running_mtx);
	}
	pthread_mutex_unlock(&running_mtx);
	if(input_enabled){
		pthread_join(tid_input,NULL);
		printf("Saimos do thread input!!!!!!\n");
		endwin_wrapper();
	}
	if(stream_show_stats){
		pthread_join(tid_stats,NULL);
		printf("Saimos do thread de stats!!!!!!\n");
		endwin_wrapper();
	}
	if(play){
		pthread_join(tid_play,NULL);
		printf("Saimos do thread play!!!!!!\n");
	}
	if(decode&&!is_wav_mode){
	
		pthread_join(tid_dec,NULL);
		printf("Saimos do thread decoder!!!!!\n");
	}
	if(rx_enabled){
		pthread_join(tid_rx,NULL);
		printf("Saimos do thread rx!!!!!\n");
	}
	pthread_join(tid_ack,NULL);
	printf("Saimos do thread ack!!!!!\n");
	
	perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_CLEAN,Q_LOOK_NA});
	if(decode&&!is_wav_mode){
		perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_CLEAN,Q_LOOK_NA});
		perform_dec_op(stream_struct.decoder,D_CLEAN,0);
	}
	stop_client_stream();
	perform_play_op(stream_struct.player,NULL,P_CLEAN);
	printf("SAIMOS DO CLIENT!\nTimeouts excedidos? %s\nVamos ver errno:%s\n",(stream_struct.curr_timeout==cfg_client_ack_timeout_lim) ? "SIM": "NAO",strerror(errno));
	return 0;
}



void player_stop_stream(void){
	raise(SIGINT);
}

void player_init_stream(con_t* con_obj,uint64_t chunk_size,method which_mode){

	init_client_stream(con_obj,chunk_size,which_mode);
}

