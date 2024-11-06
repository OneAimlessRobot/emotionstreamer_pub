#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
char client_logs_file_name[PATHSIZE]={0};
char client_music_folder_path[PATHSIZE]={0};

//EM BYTES E HZ!
u_int64_t cfg_latency_ms=DEF_LATENCY_MS,
	cfg_stream_cache_size_chunks=STREAM_CACHE_SIZE_CHUNKS,
        cfg_freq=FREQ;

uint16_t cfg_cache_almost_full_pct=CACHE_ALMOST_FULL_PCT,
	cfg_cache_almost_empty_pct=CACHE_ALMOST_EMPTY_PCT,
	cfg_client_ack_timeout_lim=CLIENT_ACK_TIMEOUT_LIM;

int_pair client_data_times_pair=(int_pair){CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC};
int_pair client_con_times_pair=(int_pair){CLIENT_TIMEOUT_CON_SEC,CLIENT_TIMEOUT_CON_USEC};


static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

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
	sscanf(curr_line_buff,"latency_ms: %lu",&cfg_latency_ms);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"cache_num_chunks: %lu",&cfg_stream_cache_size_chunks);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"cache_almost_full_pct: %hu",&cfg_cache_almost_full_pct);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"cache_almost_empty_pct: %hu",&cfg_cache_almost_empty_pct);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"freq: %lu",&cfg_freq);
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
	sscanf(curr_line_buff,"client_ack_timeout_lim: %hu",&cfg_client_ack_timeout_lim);

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

	fclose(cfg_fp);



}

void print_values_cfg_client(int fd){

	
	dprintf(fd,"latency_ms: %lu\n",cfg_latency_ms);

	dprintf(fd,"cache_almost_full_pct: %hu\n",cfg_cache_almost_full_pct);

	dprintf(fd,"cache_almost_empty_pct: %hu\n",cfg_cache_almost_empty_pct);

	dprintf(fd,"freq: %lu\n",cfg_freq);

	dprintf(fd,"client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	dprintf(fd,"client_timeouts_con: %lus %lu us\n",client_con_times_pair[0],client_con_times_pair[1]);

	dprintf(fd,"client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	dprintf(fd,"client_ack_timeout_lim: %hu",cfg_client_ack_timeout_lim);

	dprintf(fd,"stream_chunk_num: %lu chunks\n",cfg_stream_cache_size_chunks);

	dprintf(fd,"client_music_folder_path: %s\n",client_music_folder_path);

	dprintf(fd,"logs_file_name: %s\n",client_logs_file_name);


}
