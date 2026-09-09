#include "../../Includes/preprocessor.h"
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <ao/ao.h>
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include <openssl/ssl.h>
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_menus.h"
#include "../Includes/terminal_mgmt.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
char client_logs_file_name[PATHSIZE]={0};
char client_music_folder_path[PATHSIZE]={0};
char song_name_global[PATHSIZE]={0};
ip_cache_entry server_ip_cache_entry={{0},0};
ip_cache_entry client_ip_cache_entry={{0},0};

char server_ip_address_buff[PATHSIZE+1]={0};
char cfg_client_device_name_if_alsa[PATHSIZE+1]={0};
char cfg_client_device_output_if_alsa[PATHSIZE+1]={0};


uint8_t cfg_client_print_config,
	cfg_client_show_splash;

//EM BYTES E HZ!
u_int64_t cfg_latency_ms=DEF_LATENCY_MS,
	cfg_show_decoder_queue_length=PRINT_SIZE,
	cfg_show_player_queue_length=PRINT_SIZE,
	cfg_stream_decoder_cache_size_chunks=STREAM_DEF_DECODE_CACHE_SIZE_CHUNKS,
	cfg_stream_player_cache_size_chunks=STREAM_DEF_PLAYER_CACHE_SIZE_CHUNKS,
	cfg_client_ack_timeout_lim=CLIENT_ACK_TIMEOUT_LIM,
	cfg_client_alsa_device_latency_if_alsa_ms=CLIENT_ALSA_LATENCY_MS_DEFAULT;

uint8_t cfg_cache_almost_full_pct=CACHE_ALMOST_FULL_PCT,
	cfg_cache_almost_empty_pct=CACHE_ALMOST_EMPTY_PCT;

uint64_t cfg_client_ack_period_us=DEF_CLIENT_ACK_PERIOD_US;

uint8_t cfg_client_logging=0;

const char	* play_thread_name="player.Filipa",
		* play_queue_name="player.que.Filipa",
	   	* play_dev_name="player.dev.Filipa",
	   	* decode_thread_name="decoder.Ester",
	   	* decode_queue_name="decode.que.Ester",
		* input_thread_name="input.Ksun",
		* stats_thread_name="stats.Adriano",
		* rx_thread_name="main.Beatriz";


uint16_t cfg_client_chunk_size=CLIENT_DEF_CHUNK_SIZE,
		server_chunk_size=CLIENT_DEF_CHUNK_SIZE;
float cfg_ui_framerate_fps=UI_DEF_FRAMERATE_FPS;
uint64_t cfg_ui_frame_period_us=UI_DEF_FRAME_PERIOD_US;
uint8_t stream_enable_ncurses=0;
uint8_t stream_show_stats=1;
uint8_t stream_show_decoder_queue=0;
uint8_t stream_show_player_queue=1;
uint8_t stream_show_frames=1;
int16_t is_wav_mode=0;
int_pair client_data_times_pair=(int_pair){CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC};
int_pair client_con_times_pair=(int_pair){CLIENT_TIMEOUT_CON_SEC,CLIENT_TIMEOUT_CON_USEC};

static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}
static void process_ip_cache_entries(void){

	parse_ip_cache_entry(server_ip_address_buff,&server_ip_cache_entry);
	parse_ip_cache_entry(port_mapper_ip_address_buff,&client_ip_cache_entry);

}

static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	printf("Saimos no leitor de cfg. do client. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_CLIENT);
	exit(-1);
}

void read_values_cfg_client(void){

	if(!(cfg_fp=fopen(CONFIG_FILE_PATH_CLIENT,"r"))){

		clean_and_exit();
	}

	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_print_config: %hhu",&cfg_client_print_config);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_show_splash: %hhu",&cfg_client_show_splash);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){
		clean_and_exit();

	}
	sscanf(curr_line_buff,"client_logging: %hhu",&cfg_client_logging);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"stream_enable_ncurses: %hhu",&stream_enable_ncurses);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"stream_show_stats: %hhu",&stream_show_stats);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"stream_show_frames: %hhu",&stream_show_frames);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"ui_framerate_fps: %f",&cfg_ui_framerate_fps);
	cfg_ui_frame_period_us=UI_FRAME_PERIOD_US(cfg_ui_framerate_fps);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"latency_ms: %lu",&cfg_latency_ms);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){
		clean_and_exit();
	}
	sscanf(curr_line_buff,"decoder_cache_num_chunks: %lu",&cfg_stream_decoder_cache_size_chunks);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){
		clean_and_exit();
	}
	sscanf(curr_line_buff,"player_cache_num_chunks: %lu",&cfg_stream_player_cache_size_chunks);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"cache_almost_full_pct: %hhu",&cfg_cache_almost_full_pct);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){
		clean_and_exit();
	}
	sscanf(curr_line_buff,"cache_almost_empty_pct: %hhu",&cfg_cache_almost_empty_pct);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"show_decoder_queue_length: %lu",&cfg_show_decoder_queue_length);
	cfg_show_decoder_queue_length=min(cfg_stream_decoder_cache_size_chunks,max(MIN_PRINT_SIZE,cfg_show_decoder_queue_length));
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	clean_and_exit();
		clean_and_exit();
	}
	sscanf(curr_line_buff,"show_player_queue_length: %lu",&cfg_show_player_queue_length);
	cfg_show_player_queue_length=min(cfg_stream_player_cache_size_chunks,max(MIN_PRINT_SIZE,cfg_show_player_queue_length));
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"show_decoder_queue: %hhu",&stream_show_decoder_queue);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"show_player_queue: %hhu",&stream_show_player_queue);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_chunk_size: %hu",&cfg_client_chunk_size);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_timeouts_con: %lu %lu",&client_con_times_pair[0],&client_con_times_pair[1]);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_timeouts_data: %lu %lu",&client_data_times_pair[0],&client_data_times_pair[1]);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	curr_line_buff[strlen(curr_line_buff)-1]=0;
	snprintf(client_music_folder_path,sizeof(client_music_folder_path),"%s",(char*)&curr_line_buff[strlen("client_music_folder_path: ")]);
        clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	curr_line_buff[strlen(curr_line_buff)-1]=0;
	snprintf(client_logs_file_name,sizeof(client_logs_file_name),"%s",(char*)&curr_line_buff[strlen("log_file_name: ")]);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_device_name_if_alsa: %s",cfg_client_device_name_if_alsa);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_device_output_if_alsa: %s",cfg_client_device_output_if_alsa);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_alsa_device_latency_if_alsa_ms: %lu",&cfg_client_alsa_device_latency_if_alsa_ms);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"server_ip_address: %s", server_ip_address_buff);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"client_using_tls: %hu", &will_use_tls);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(will_use_tls){
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

			clean_and_exit();
		}
		curr_line_buff[strlen(curr_line_buff)-1]=0;
		snprintf(auth_cert_file_path,sizeof(auth_cert_file_path),"%s",(char*)&curr_line_buff[strlen("client_auth_cert_path: ")]);
		clean_buff();
		skip_config_comments(cfg_fp);
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

			clean_and_exit();
		}
		curr_line_buff[strlen(curr_line_buff)-1]=0;
		snprintf(host_cert_file_path,sizeof(host_cert_file_path),"%s",(char*)&curr_line_buff[strlen("client_host_cert_path: ")]);
		clean_buff();
		skip_config_comments(cfg_fp);
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

			clean_and_exit();
		}
		curr_line_buff[strlen(curr_line_buff)-1]=0;
		snprintf(host_pkey_file_path,sizeof(host_pkey_file_path),"%s",(char*)&curr_line_buff[strlen("client_host_pkey_path: ")]);
		clean_buff();
	}
	fclose(cfg_fp);
	process_ip_cache_entries();

}

void print_values_cfg_client(int fd){

	dprintf(fd,"client_print_config: %hhu\n",cfg_client_print_config);

	dprintf(fd,"client_show_splash: %hhu\n",cfg_client_show_splash);

	dprintf(fd,"client_logging: %hhu\n",cfg_client_logging);

	dprintf(fd,"stream_enable_ncurses: %s\n",stream_enable_ncurses?"Yes":"No");

	dprintf(fd,"stream_show_stats: %s\n",stream_show_stats?"Yes":"No");

	dprintf(fd,"stream_show_frames: %s\n",stream_show_frames?"Yes":"No");

	dprintf(fd,"ui_framerate_fps (frame period in us): %f (%lu us)\n",cfg_ui_framerate_fps,cfg_ui_frame_period_us);

	dprintf(fd,"latency_ms: %lu\n",cfg_latency_ms);

	dprintf(fd,"cache_almost_full_pct: %hhu\n",cfg_cache_almost_full_pct);

	dprintf(fd,"cache_almost_empty_pct: %hhu\n",cfg_cache_almost_empty_pct);

	dprintf(fd,"show_decoder_queue_length: %lu\n",cfg_show_decoder_queue_length);

	dprintf(fd,"show_player_queue_length: %lu\n",cfg_show_player_queue_length);

	dprintf(fd,"show_decoder_queue: %hhu\n",stream_show_decoder_queue);

	dprintf(fd,"show_player_queue: %hhu\n",stream_show_player_queue);

	dprintf(fd,"client_chunk_size: %hu\n",cfg_client_chunk_size);

	dprintf(fd,"client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	dprintf(fd,"client_timeouts_con: %lus %lu us\n",client_con_times_pair[0],client_con_times_pair[1]);

	dprintf(fd,"decoder_cache_num_chunks: %lu\n",cfg_stream_decoder_cache_size_chunks);

	dprintf(fd,"player_cache_num_chunks: %lu\n",cfg_stream_player_cache_size_chunks);

	dprintf(fd,"client_music_folder_path: %s\n",client_music_folder_path);

	dprintf(fd,"logs_file_name: %s\n",client_logs_file_name);

	dprintf(fd,"client_device_name_if_alsa: %s\n",cfg_client_device_name_if_alsa);

	dprintf(fd,"client_device_output_if_alsa: %s\n",cfg_client_device_output_if_alsa);

	dprintf(fd,"client_using_tls: %hu\n",will_use_tls);

        if(will_use_tls){

                dprintf(fd,"client_auth_cert_path: %s\n", auth_cert_file_path);

                dprintf(fd,"client_host_cert_path: %s\n", host_cert_file_path);

                dprintf(fd,"client_pkey_cert_path: %s\n", host_pkey_file_path);

        }

	dprintf(fd,"client_alsa_device_latency_if_alsa_ms: %lu ms (%lu us)\n",
				cfg_client_alsa_device_latency_if_alsa_ms,
				MS_TO_US(cfg_client_alsa_device_latency_if_alsa_ms));

	print_ip_cache_entry(stdout,&server_ip_cache_entry);
}
