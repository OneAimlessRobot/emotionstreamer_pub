#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/terminal_mgmt.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
char client_logs_file_name[PATHSIZE]={0};
char client_music_folder_path[PATHSIZE]={0};
char generalized_config_filepath_buff[PATHSIZE+1]={0};
ip_cache_entry server_ip_cache_entry={{0},0};
ip_cache_entry client_ip_cache_entry={{0},0};
ip_cache_entry client_port_mapper_ip_cache_entry={{0},0};//

char server_ip_address_buff[PATHSIZE+1]={0};
char client_ip_address_buff[PATHSIZE+1]={0};
char client_port_mapper_ip_address_buff[PATHSIZE+1]={0};


//EM BYTES E HZ!
u_int64_t cfg_latency_ms=DEF_LATENCY_MS,
	cfg_stream_decoder_cache_size_chunks=STREAM_DEF_DECODE_CACHE_SIZE_CHUNKS,
	cfg_stream_player_cache_size_chunks=STREAM_DEF_PLAYER_CACHE_SIZE_CHUNKS,
	cfg_client_ack_timeout_lim=CLIENT_ACK_TIMEOUT_LIM;

uint8_t cfg_cache_almost_full_pct=CACHE_ALMOST_FULL_PCT,
	cfg_cache_almost_empty_pct=CACHE_ALMOST_EMPTY_PCT;

uint64_t cfg_client_ack_period_us=DEF_CLIENT_ACK_PERIOD_US;

uint16_t cfg_client_chunk_size=CLIENT_DEF_CHUNK_SIZE;
float cfg_ui_framerate_fps=UI_DEF_FRAMERATE_FPS;
uint64_t cfg_ui_frame_period_us=UI_DEF_FRAME_PERIOD_US;
int8_t streaming_protocol=0;
uint8_t stream_enable_ncurses=0;
uint8_t stream_show_stats=1;
uint8_t stream_show_decoder_queue=0;
uint8_t stream_show_player_queue=1;
uint8_t stream_show_frames=1;
int8_t is_wav_mode=0;
int_pair client_data_times_pair=(int_pair){CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC};
int_pair client_con_times_pair=(int_pair){CLIENT_TIMEOUT_CON_SEC,CLIENT_TIMEOUT_CON_USEC};
int_pair client_ack_times_pair=(int_pair){CLIENT_TIMEOUT_ACK_SEC,CLIENT_TIMEOUT_ACK_USEC};
int_pair client_holepunching_times_pair=(int_pair){HOLE_PUNCHING_TIMEOUT_SEC,HOLE_PUNCHING_TIMEOUT_USEC};

static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}
static void process_ip_cache_entries(void){

	parse_ip_cache_entry(server_ip_address_buff,&server_ip_cache_entry);
	parse_ip_cache_entry(client_ip_address_buff,&client_ip_cache_entry);
	parse_ip_cache_entry(client_port_mapper_ip_address_buff,&client_port_mapper_ip_cache_entry);

}

static void sigint_handler(int useless){
	
	printf("Saimos no leitor de cfg. do client. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_CLIENT);
	exit(useless);
}

void read_values_cfg_client(void){
	
	signal(SIGINT,sigint_handler);

	if(!(cfg_fp=fopen(CONFIG_FILE_PATH_CLIENT,"r"))){

		raise(SIGINT);
	}
	
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"stream_enable_ncurses: %hhu",&stream_enable_ncurses);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"stream_show_stats: %hhu",&stream_show_stats);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"stream_show_frames: %hhu",&stream_show_frames);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"ui_framerate_fps: %f",&cfg_ui_framerate_fps);
	cfg_ui_frame_period_us=UI_FRAME_PERIOD_US(cfg_ui_framerate_fps);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"latency_ms: %lu",&cfg_latency_ms);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"decoder_cache_num_chunks: %lu",&cfg_stream_decoder_cache_size_chunks);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"player_cache_num_chunks: %lu",&cfg_stream_player_cache_size_chunks);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"cache_almost_full_pct: %hhu",&cfg_cache_almost_full_pct);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"cache_almost_empty_pct: %hhu",&cfg_cache_almost_empty_pct);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"show_decoder_queue: %hhu",&stream_show_decoder_queue);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"show_player_queue: %hhu",&stream_show_player_queue);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_chunk_size: %hu",&cfg_client_chunk_size);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_timeouts_con: %lu %lu",&client_con_times_pair[0],&client_con_times_pair[1]);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_timeouts_data: %lu %lu",&client_data_times_pair[0],&client_data_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_timeouts_ack: %lu %lu",&client_ack_times_pair[0],&client_ack_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_ack_period_us: %lu",&cfg_client_ack_period_us);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_timeouts_holepunching: %lu %lu",&client_holepunching_times_pair[0],&client_holepunching_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_ack_timeout_lim: %lu",&cfg_client_ack_timeout_lim);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"client_music_folder_path: %s",client_music_folder_path);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"log_file_name: %s",client_logs_file_name);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_ip_address: %s", server_ip_address_buff);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
//client_port_mapper_ip_address:
	sscanf(curr_line_buff,"client_ip_address: %s", client_ip_address_buff);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
//
	sscanf(curr_line_buff,"client_port_mapper_ip_address: %s", client_port_mapper_ip_address_buff);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"generalized_config_filepath: %s",generalized_config_filepath_buff);
        clean_buff();
	fclose(cfg_fp);

	process_ip_cache_entries();

}

void print_values_cfg_client(int fd){

	
	dprintf(fd,"stream_enable_ncurses: %s\n",stream_enable_ncurses?"Yes":"No");

	dprintf(fd,"stream_show_stats: %s\n",stream_show_stats?"Yes":"No");

	dprintf(fd,"stream_show_frames: %s\n",stream_show_frames?"Yes":"No");

	dprintf(fd,"ui_framerate_fps (frame period in us): %f (%lu us)\n",cfg_ui_framerate_fps,cfg_ui_frame_period_us);

	dprintf(fd,"latency_ms: %lu\n",cfg_latency_ms);

	dprintf(fd,"cache_almost_full_pct: %hhu\n",cfg_cache_almost_full_pct);

	dprintf(fd,"cache_almost_empty_pct: %hhu\n",cfg_cache_almost_empty_pct);

	dprintf(fd,"show_decoder_queue: %hhu\n",stream_show_decoder_queue);

	dprintf(fd,"show_player_queue: %hhu\n",stream_show_player_queue);

	dprintf(fd,"client_chunk_size: %hu\n",cfg_client_chunk_size);

	dprintf(fd,"client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	dprintf(fd,"client_timeouts_con: %lus %lu us\n",client_con_times_pair[0],client_con_times_pair[1]);

	dprintf(fd,"client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	dprintf(fd,"client_timeouts_ack: %lus %lu us\n",client_ack_times_pair[0],client_ack_times_pair[1]);

	dprintf(fd,"client_ack_period_us: %luus\n",cfg_client_ack_period_us);

	dprintf(fd,"client_timeouts_holepunching: %lu %lu\n",client_holepunching_times_pair[0],client_holepunching_times_pair[1]);

	dprintf(fd,"client_ack_timeout_lim: %lu\n",cfg_client_ack_timeout_lim);

	dprintf(fd,"decoder_cache_num_chunks: %lu\n",cfg_stream_decoder_cache_size_chunks);

	dprintf(fd,"player_cache_num_chunks: %lu\n",cfg_stream_player_cache_size_chunks);

	dprintf(fd,"client_music_folder_path: %s\n",client_music_folder_path);

	dprintf(fd,"logs_file_name: %s\n",client_logs_file_name);

        dprintf(fd,"generalized_config_filepath: %s\n",generalized_config_filepath_buff);

	print_ip_cache_entry(stdout,&server_ip_cache_entry);

	print_ip_cache_entry(stdout,&client_ip_cache_entry);

	print_ip_cache_entry(stdout,&client_port_mapper_ip_cache_entry);

}
