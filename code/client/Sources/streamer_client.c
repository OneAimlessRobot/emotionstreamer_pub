#include "../Includes/preprocessor.h"
#include <sys/syscall.h>
#include <ncurses.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <ao/ao.h>
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_menus.h"
#include "../Includes/mp3module.h"
#include "../Includes/chunk_player.h"
#include "../Includes/streamer_client.h"
#include "../Includes/terminal_mgmt.h"
#include "../Includes/client_aux_funcs.h"

static uint8_t read_tcp_chk[sizeof(mp3_stream_chunk)]={0};
static clock_t stats_start, stats_end;
static float stats_cpu_time_used,
	stats_time_diff;

static const int play=1;
static const int decode=1;
static const int input_enabled=1;
static struct sigaction sa;
static struct sigaction sa_winch;

static char decode_print_buff[DEF_DATASIZE]={0},
		stats_print_buff[DEF_DATASIZE*2+1]={0},
		input_buff[DEF_DATASIZE+1]={0},
		error_buff_decoder[DEF_DATASIZE]={0},
		error_buff_stats[DEF_DATASIZE]={0},
		error_buff_play[DEF_DATASIZE]={0},
		error_buff_input[DEF_DATASIZE]={0},
		error_buff_rx[DEF_DATASIZE]={0};

static decoder_result_struct stats_result_struct={0};

static pthread_cond_t reading_cond=PTHREAD_COND_INITIALIZER,
	       player_cond=PTHREAD_COND_INITIALIZER,
	       running_cond=PTHREAD_COND_INITIALIZER,
	       input_cond=PTHREAD_COND_INITIALIZER,
	       stats_cond=PTHREAD_COND_INITIALIZER,
	       decoder_cond=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t reading_mtx=PTHREAD_MUTEX_INITIALIZER,
		player_mtx=PTHREAD_MUTEX_INITIALIZER,
		running_mtx=PTHREAD_MUTEX_INITIALIZER,
		variable_acess_mtx=PTHREAD_MUTEX_INITIALIZER,
		input_mtx=PTHREAD_MUTEX_INITIALIZER,
		stats_mtx=PTHREAD_MUTEX_INITIALIZER,
		exit_mtx=PTHREAD_MUTEX_INITIALIZER,
		decoder_mtx=PTHREAD_MUTEX_INITIALIZER;


static pthread_t t_play,
	  	t_dec,
	  	t_input,
	  	t_stats;

/*

Not needed, for now
But leave them here
Lets save just a wee bit of memory
cuz why not

static pid_t tid_rx,
		tid_play,
	  	tid_dec,
	  	tid_input,
	  	tid_stats;

*/

static atomic_int lost_packet=0,
		reading=0,
		decoding=0,
		playing=0,
		showing=0,
		input_thread_alive=1,
		terminating=0,
		paused=0,
		ready_2_go=0,
		is_first_player_chunk=1,
		we_got_the_stats=0,
		stats_time_ms=0,
		decode_queue_empty=0,
		play_queue_full=0,
		decode_ret_val=MPG123_NEED_MORE,
		read_result=-1,
		decode_queue_full=0,
		pause_value=0,
		rx_result=1,
		play_queue_empty=0;


#define READ_RESULT_INVALID(a) ((a<=0)&&(a!=(-2)))

#define READ_RESULT_TIMEOUT(a) ((a==(-2)))


static atomic_int innited=0;
static atomic_int exiting=0;
static atomic_int will_redraw=0;

static chunk_queue player_que={0},
		decoder_que={0};
static chunk_player player={0};
static decoder_t decoder={0};

static client_stream_t stream_struct={
					0,
					NULL,
					NULL,
                                        NULL,
					NULL,
                                        NULL
                                        };

void stop_client_stream(void){

	innited=0;
        reading=0;
	paused=0;
	decoding=1;
	playing=1;
	pthread_cond_signal(&running_cond);
	pthread_cond_signal(&player_cond);
	pthread_cond_signal(&decoder_cond);
	pthread_cond_signal(&reading_cond);
	pthread_cond_signal(&input_cond);
	pthread_cond_signal(&stats_cond);

}
static void sigint_handler(int useless){

        innited=0*useless;
	reading=0;
	paused=0;
	input_thread_alive=0;
	terminating=1;
	decoding=1;
	playing=1;
	pthread_cond_signal(&running_cond);
	pthread_cond_signal(&player_cond);
	pthread_cond_signal(&decoder_cond);
	pthread_cond_signal(&reading_cond);
	pthread_cond_signal(&input_cond);
	pthread_cond_signal(&stats_cond);

}
static void sigwinch_handler(int useless){

	will_redraw=1+(0*useless);

}
static int is_wav_compat_mode(void){

	return (is_wav_mode||!decode);

}
/*

size of buff must be DEF_DATASIZE!

*/
static void set_thread_name_wrapper(char *error_buff, const char* the_name_to_set){
	int set_name_result = set_this_thread_name(the_name_to_set);

	if(logging){
		(set_name_result ?
		snprintf(error_buff,DEF_DATASIZE-1, "Error setting thread name! %s\n",strerror(errno))
						:
		snprintf(error_buff,DEF_DATASIZE-1, "Sucessfully set thread name to %s\n", the_name_to_set));

		print_log_string(error_buff);
	}
}
static int read_chunk(client_stream_t* strm,int_pair pair){
	if(is_first_player_chunk){
		if(strm->con_obj->is_ssl){
			read_result= readsome_ssl(strm->con_obj->con_ssl,(char*)(strm->player->h_chunk),strm->player->chunk_size,pair);
		}
		else{

			read_result= readsome(strm->con_obj->sockfd_tcp,(char*)(strm->player->h_chunk),strm->player->chunk_size,pair,0);
		}
	}
	else{
		if(strm->con_obj->is_ssl){
			read_result= readsome_ssl(strm->con_obj->con_ssl,(char*)(is_wav_compat_mode()?strm->player->r_chunk:read_tcp_chk),is_wav_compat_mode()?strm->player->chunk_size:strm->decoder->d_chunk_size,pair);
		}
		else{
			read_result= readsome(strm->con_obj->sockfd_tcp,(char*)(is_wav_compat_mode()?strm->player->r_chunk:read_tcp_chk),is_wav_compat_mode()?strm->player->chunk_size:strm->decoder->d_chunk_size,pair,0);
		}
		if(decode&&!is_wav_mode){
			memcpy(strm->decoder->r_chunk,&read_tcp_chk,strm->decoder->d_chunk_size);
		}
	}
	if(read_result<0){
		lost_packet=1;
		return read_result;
	}
	lost_packet=0;

	if(is_first_player_chunk){
		perform_play_op(stream_struct.player,NULL,P_INIT_LIBS);
		is_first_player_chunk=0;
	}
	else{
		perform_queue_op(is_wav_compat_mode()?strm->player_que:strm->decoder_que,is_wav_compat_mode()?strm->player->r_chunk:strm->decoder->r_chunk,NULL,(q_op){Q_READ_FROM,Q_LOOK_NA});
	}
	return read_result;
}


static void* rx_thread_func(void *args){
	print_log_string("Thread de reading alcançado!\n");
	set_thread_name_wrapper(error_buff_rx,rx_thread_name);
	while(innited){
		ready_2_go=1;
		reading=1;
		if(!is_wav_compat_mode()){
			print_log_string("Lets poke the decoder thread!\n");
			pthread_cond_signal(&decoder_cond);
			print_log_string("Decoder thread poked!\n");
		}
		while(innited){
			if(paused){
				break;
			}
			rx_result=read_chunk(&stream_struct,client_data_times_pair);
			if(READ_RESULT_INVALID(rx_result)){

				print_string(rx_result?"Thread de reading parado (early)!!!\n":"Server parou de enviar dados! Encerrando...\n");
				return args;

			}
			if(READ_RESULT_INVALID(rx_result)){

				print_log_string("Timeout in rx (reader) thread!\n");
				continue;

			}
			if(!is_wav_compat_mode()){
				pthread_cond_signal(&decoder_cond);
			}
			else if(!is_first_player_chunk){

				pthread_cond_signal(&player_cond);
			}
			if(input_enabled){
				pthread_cond_signal(&input_cond);
			}
			decode_queue_full=perform_queue_op(is_wav_compat_mode()?stream_struct.player_que:stream_struct.decoder_que,NULL,NULL,is_wav_compat_mode()?(q_op){Q_LOOK,Q_IS_FULL}:(q_op){Q_LOOK,Q_IS_FULL});
			if(decode_queue_full){
				if(!is_wav_compat_mode()){
					print_log_string("Lets poke the decoder thread!\n");
					pthread_cond_signal(&decoder_cond);
					print_log_string("Decoder thread poked!\n");
				}
				print_log_string("Breaking loop due to full decoding/playing queue!\n");
				break;
			}
		}
		pthread_mutex_lock(&reading_mtx);
		while(innited&&(paused||(perform_queue_op(is_wav_compat_mode()?stream_struct.player_que:stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,is_wav_compat_mode()?Q_IS_FULL:Q_IS_ALMOST_FULL})))){

			reading=0;
			pthread_cond_wait(&reading_cond,&reading_mtx);

		}
		pthread_mutex_unlock(&reading_mtx);
	}
	print_log_string("Thread de reading parado!!!\n");
	return args;
}

static void* dec_thread_func(void* args){
	pthread_mutex_lock(&decoder_mtx);
	while(!ready_2_go&&innited){
		print_log_string("Waiting loop de thread de decoding alcançado!\n");
		pthread_cond_wait(&decoder_cond,&decoder_mtx);
		print_log_string("Waiting loop de thread de decoding triggered com cond!\n");
	}
	pthread_mutex_unlock(&decoder_mtx);
	set_thread_name_wrapper(error_buff_decoder,decode_thread_name);
	print_log_string("Thread de decoding alcançado!\n");
	while(innited){
		decoding=1;
		while(innited){
			perform_queue_op(stream_struct.decoder_que,stream_struct.decoder->d_chunk,NULL,(q_op){Q_READ_TO,Q_LOOK_NA});
			decode_ret_val=perform_dec_op(stream_struct.decoder,D_DECODE_CHUNK,DO_DECODE);
			if(((mp3_processed_chunk*)stream_struct.decoder->p_chunk)->result_struct.decoder_state){
				perform_dec_op(stream_struct.decoder,D_RESET_BOTH,DO_DECODE);
				perform_queue_op(stream_struct.player_que,stream_struct.decoder->p_chunk,NULL,(q_op){Q_READ_FROM,Q_LOOK_NA});
				pthread_cond_signal(&player_cond);
				play_queue_full=perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_FULL});
				if(play_queue_full){
					break;
				}
			}
			pthread_cond_signal(&reading_cond);
			if(!(decode_ret_val==MPG123_NEED_MORE)){
				memset(decode_print_buff,0,sizeof(decode_print_buff));
				if(decode_ret_val==MPG123_DONE){
					snprintf(decode_print_buff,sizeof(decode_print_buff)-1,"Stream done!\n");
					print_log_string(decode_print_buff);
					goto decoder_exit;
				}
				else if(decode_ret_val==MPG123_ERR){
					snprintf(decode_print_buff,sizeof(decode_print_buff)-1,"Decoding error: %s\n",mpg123_strerror(stream_struct.decoder->dec));
					print_log_string(decode_print_buff);
					goto decoder_exit;
				}
			}
			if(paused){
				break;
			}
			decode_queue_empty=perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_EMPTY});
			if(decode_queue_empty){
				if(READ_RESULT_INVALID(rx_result)){
					goto decoder_exit;
				}
				break;
			}

	}
	pthread_mutex_lock(&decoder_mtx);

	while(innited&&(paused||perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_ALMOST_FULL})||perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_ALMOST_EMPTY}))){

		decoding=0;
		pthread_cond_wait(&decoder_cond,&decoder_mtx);
	}
	pthread_mutex_unlock(&decoder_mtx);
	}
	decoder_exit:
		print_log_string("Thread de decoding parado!!!\n");
		return  args;
}

static void* play_thread_func(void* args){

	pthread_mutex_lock(&player_mtx);
	while(!is_wav_compat_mode()?!decoding:!reading&&innited){
		pthread_cond_wait(&player_cond,&player_mtx);
	}
	pthread_mutex_unlock(&player_mtx);
	print_log_string("Thread de play alcançado!\n");
	set_thread_name_wrapper(error_buff_play,play_thread_name);
	usleep(1000*1000);
	while(innited){
		playing=1;
		while(innited){
			if(is_first_player_chunk){
				continue;
			}
			if(paused){
				break;
			}
			perform_queue_op(stream_struct.player_que,is_wav_compat_mode()?stream_struct.player->pr_chunk:stream_struct.player->p_chunk,NULL,(q_op){Q_READ_TO,Q_LOOK_NA});
			showing=1;
			pthread_cond_signal(&stats_cond);
			if(!is_wav_compat_mode()){
				perform_play_op(stream_struct.player,NULL,P_SAFE_DECODER_BUFF_LOAD);
				pthread_cond_signal(&decoder_cond);
			}
			else{

				pthread_cond_signal(&reading_cond);
			}
			play_queue_empty=perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_EMPTY});
			if(play_queue_empty){
				if(READ_RESULT_INVALID(rx_result)){
					goto end_player;
				}
				break;
			}
			perform_play_op(stream_struct.player,NULL,P_REAL_PLAY);
	}
	pthread_mutex_lock(&player_mtx);
	while(innited&&(paused||perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_LOOK,Q_IS_ALMOST_EMPTY}))){
		playing=0;
		pthread_cond_wait(&player_cond,&player_mtx);
	}
	pthread_mutex_unlock(&player_mtx);
	}
	end_player:
		print_log_string("Thread de playing parado!!!\n");
		input_thread_alive=0;
		terminating=1;
		return  args;
}
static void* show_stats(void* args){

	pthread_mutex_lock(&stats_mtx);
	while(!showing&&innited){
		pthread_cond_wait(&stats_cond,&stats_mtx);
	}
	pthread_mutex_unlock(&stats_mtx);
	set_thread_name_wrapper(error_buff_stats,stats_thread_name);
	print_string("Thread de stats alcançado!\n");
	if(stream_enable_ncurses){
        	enable_ncurses();
		clearok(stdscr,1);
		touchwin(stdscr);
		refresh();
		doupdate();
	}
	else{
		printf("\033[2J");

	}
	while(innited){
	        stats_start = clock();
		if(stream_enable_ncurses){
			clear();
		}
		else{
			if(will_redraw){
				printf("\033[2J");
				will_redraw=0;
			}
			printf("\033[H");
		}
		memset(stats_print_buff,0,sizeof(stats_print_buff)-1);
	        char* stats_print_buff_ptr=stats_print_buff;
		if(!we_got_the_stats){
			perform_play_op(stream_struct.player,&stats_result_struct,P_GET_FRAME_DATA);
			we_got_the_stats= (stats_result_struct.nsamples>0);
		}
		if(we_got_the_stats){
			stats_time_ms=perform_queue_op(stream_struct.player_que,NULL,&stats_result_struct,(q_op){Q_GET_TIME,Q_LOOK_NA});
		}
		stats_print_buff_ptr+=snprintf(stats_print_buff_ptr,(sizeof(stats_print_buff)-1)-(stats_print_buff_ptr-stats_print_buff),
					"Song name: %s\n\n",
					song_name_global);

		stats_print_buff_ptr+=(we_got_the_stats?
				snprintf(stats_print_buff_ptr,(sizeof(stats_print_buff)-1)-(stats_print_buff_ptr-stats_print_buff),
					"buffer: %d ms\n",
					stats_time_ms)
						:
				snprintf(stats_print_buff_ptr,(sizeof(stats_print_buff)-1)-(stats_print_buff_ptr-stats_print_buff),
					"We have no timing data, at this point\n"));

		stats_print_buff_ptr+=snprintf(stats_print_buff_ptr,(sizeof(stats_print_buff)-1)-(stats_print_buff_ptr-stats_print_buff),
					"Reading?: %sDecoding?: %s Playing?: %s Paused?: %s\nWAV innited? %s\n\n",
					reading ? "READING ": "    ",
					decoding ? "DECODING ": "    ",
					playing ? "PLAYING ": "    ",
					paused ? "PAUSED ": "    ",
					is_wav_mode?(is_first_player_chunk ? "YES! ": "NO..."):"Not in WAV mode...");

		print_string(stats_print_buff);
		if(stream_show_decoder_queue&&decode&&!is_wav_mode){
			perform_queue_op(stream_struct.decoder_que,NULL,&stats_result_struct,(q_op){Q_PRINT,Q_LOOK_NA});
		}
		if(stream_show_player_queue){
			perform_queue_op(stream_struct.player_que,NULL,&stats_result_struct,(q_op){Q_PRINT,Q_LOOK_NA});
		}
		if(stream_enable_ncurses){
			touchwin(stdscr);
			wrefresh(stdscr);
			fflush(stdout);
		}
		stats_end = clock();
	        stats_cpu_time_used = F_S_TO_US(((float) (stats_end - stats_start)) / CLOCKS_PER_SEC);
		stats_time_diff=((float)cfg_ui_frame_period_us)-stats_cpu_time_used;
		if(stats_time_diff>0.0){
			usleep((uint64_t)roundf(stats_time_diff));
		}
	}
	return args;
}

static void* input_thread_func(void* args){

	pthread_mutex_lock(&input_mtx);
        while(!is_wav_compat_mode()?!decoding:!reading&&innited){
                pthread_cond_wait(&input_cond,&input_mtx);
        }
	pthread_mutex_unlock(&input_mtx);
	set_thread_name_wrapper(error_buff_input,input_thread_name);
	print_log_string("Thread de input alcançado!\n");
	struct termios ttystate, ttysave;


	if(!stream_enable_ncurses){
		//get the terminal state
		tcgetattr(STDIN_FILENO, &ttystate);
		ttysave = ttystate;
		//turn off canonical mode and echo
		ttystate.c_lflag &= ~(ICANON | ECHO);
		//minimum of number input read.
		ttystate.c_cc[VMIN] = 2;
	}
	while(input_thread_alive){
		memset(input_buff,0,sizeof(input_buff)-1);
		if(stream_enable_ncurses){
			input_buff[0]=getchar();
		}
		else{
			int_pair the_pair={3,0};
			getsome(0,input_buff,2,the_pair);
			fflush(stdin);
		}
		switch(input_buff[0]){

			case 'p':
				pause_value=paused;
				if(pause_value){

					pthread_cond_signal(&reading_cond);
					pthread_cond_signal(&player_cond);
				}
				paused=!pause_value;
			break;
			case 's':
				pthread_mutex_lock(&input_mtx);
				print_log_string("Tentando sair!\n");
				input_thread_alive=0;
				stop_client_stream();
				pthread_mutex_unlock(&input_mtx);
			break;
			default:
				print_log_string("No input!\n");
			break;
		}


	}
	if(!stream_enable_ncurses){
		ttystate.c_lflag |= ICANON | ECHO;
		//set the terminal attributes.
		tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
		// report success
	}
	fflush(stdin);
	terminating=1;
	stop_client_stream();
	return args;

}
static int init_client_stream(con_t* con_obj, uint16_t chunk_size,method which_mode){
        sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGPIPE, &sa, NULL);

	if(!stream_enable_ncurses){
	        sa_winch.sa_handler = sigwinch_handler;
        	sigemptyset(&sa_winch.sa_mask);
        	sa_winch.sa_flags = SA_RESTART;
        	sigaction(SIGWINCH, &sa_winch, NULL);
	}
	uint8_t h_chunk_buff[is_wav_compat_mode()?chunk_size:1];
	memset(h_chunk_buff,0,sizeof(h_chunk_buff));
	uint8_t r_chunk_buff[sizeof(frame_info_t)+4+chunk_size];
	memset(r_chunk_buff,0,sizeof(r_chunk_buff));
	uint8_t d_chunk_buff[is_wav_compat_mode()?1:sizeof(frame_info_t)+4+chunk_size];
	memset(d_chunk_buff,0,sizeof(d_chunk_buff));
	uint8_t pd_chunk_buff[is_wav_compat_mode()?chunk_size:sizeof(decoder_result_struct)+4+cfg_client_chunk_size];
	memset(pd_chunk_buff,0,sizeof(pd_chunk_buff));
	uint8_t pp_chunk_buff[sizeof(pd_chunk_buff)];
	memset(pp_chunk_buff,0,sizeof(pp_chunk_buff));
	uint8_t pr_chunk_buff[(is_wav_compat_mode()?chunk_size:cfg_client_chunk_size)];
	memset(pr_chunk_buff,0,sizeof(pr_chunk_buff));
	init_queue(&player_que,sizeof(pd_chunk_buff),cfg_stream_player_cache_size_chunks,cfg_show_player_queue_length,(char*)play_queue_name);
	init_chunk_player(&player,sizeof(pp_chunk_buff),sizeof(pr_chunk_buff),h_chunk_buff,r_chunk_buff,pp_chunk_buff,pr_chunk_buff,which_mode);
	stream_struct.player_que=&player_que;
	stream_struct.player=&player;
	if(!is_wav_compat_mode()){
		printf("Decoder will be initialized\n");
		init_queue(&decoder_que,sizeof(d_chunk_buff),cfg_stream_decoder_cache_size_chunks,cfg_show_decoder_queue_length,(char*)decode_queue_name);
		init_decoder(&decoder,sizeof(d_chunk_buff),sizeof(pd_chunk_buff),r_chunk_buff,d_chunk_buff,pd_chunk_buff);
		stream_struct.decoder=&decoder;
		stream_struct.decoder_que=&decoder_que;
		is_first_player_chunk=0;
		printf("Decoder thread initialized successfully\n");
	}
	stream_struct.con_obj=con_obj;
	innited=1;
	if(play){
		printf("Player thread (named %s) to be initialized\n",play_thread_name);
		create_client_thread(&t_play,play_thread_func);
		printf("Player thread (named %s) initialized sucessfully\n",play_thread_name);
	}
	if(input_enabled){
		printf("Input thread (named %s) to be initialized\n",input_thread_name);
		create_client_thread(&t_input,input_thread_func);
		printf("Input thread (named %s) initialized successfully\n",input_thread_name);
	}
	if(!is_wav_compat_mode()){
		printf("Decoder thread (named %s) to be initialized\n",decode_thread_name);
		create_client_thread(&t_dec,dec_thread_func);
		printf("Decoder thread (named %s) initialized successfully\n",decode_thread_name);
	}
	if(stream_show_stats){
		printf("Stats thread (named %s) to be initialized\n",stats_thread_name);
		create_client_thread(&t_stats,show_stats);
		printf("Stats thread (named %s) initialized sucessfully\n",stats_thread_name);
	}

	/*


		https://stackoverflow.com/questions/2218159/how-to-set-c-console-window-title

	*/

	//printf("\033]0;%s%s\007", "Emotionstreamer Client - ", song_name_global );
	//printf("\033]0;%s\007", "Emotionstreamer Client");
	printf("\033]0;%s(%s)\007", "Emotionstreamer Client - ", song_name_global );
	rx_thread_func(NULL);
	pthread_mutex_lock(&running_mtx);
	while(innited&&!terminating){
		pthread_cond_wait(&running_cond,&running_mtx);
	}
	pthread_mutex_unlock(&running_mtx);
	if(stream_show_stats){
		join_client_thread(t_stats,(char*)stats_thread_name);
	}
	if(input_enabled){
		join_client_thread(t_input,(char*)input_thread_name);
		endwin_wrapper();
	}
	if(decode&&!is_wav_mode){
		join_client_thread(t_dec,(char*)decode_thread_name);
	}
	if(play){
	#if EMSTREAM_ON_TERMUX
		join_client_thread(t_play,(char*)play_thread_name);
	#else
		join_client_thread_with_timeout(t_play,(char*)play_thread_name);
	#endif
	}
	perform_queue_op(stream_struct.player_que,NULL,NULL,(q_op){Q_CLEAN,Q_LOOK_NA});
	if(!is_wav_compat_mode()){
		perform_queue_op(stream_struct.decoder_que,NULL,NULL,(q_op){Q_CLEAN,Q_LOOK_NA});
		perform_dec_op(stream_struct.decoder,D_CLEAN,0);
	}
	perform_play_op(stream_struct.player,NULL,P_CLEAN);
	pthread_mutex_lock(&exit_mtx);
	if(!exiting&&acess_var_mtx(&variable_acess_mtx,&stream_struct.con_obj->is_on,0,V_LOOK)){
		send_port_back(htons(stream_struct.con_obj->this_tcp_addr.sin_port),&port_mapper_ip_cache_entry);
		close_con(stream_struct.con_obj,1,1);
		exiting=1;
	}
	pthread_mutex_unlock(&exit_mtx);
	end_openssl_libs_client_side();
	printf("SAIMOS DO CLIENT!\n");
	return 0;
}

void exit_emergency_func_stream(void){

	stop_client_stream();
}


void player_init_stream(con_t* con_obj,uint64_t chunk_size,method which_mode){

	exit_func_for_this_module=exit_emergency_func_stream;
	init_client_stream(con_obj,chunk_size,which_mode);
}

