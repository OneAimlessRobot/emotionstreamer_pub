#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/streamer_const.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

char server_music_folder_path[PATHSIZE]={0};

//EM BYTES E HZ!
u_int64_t cfg_chunk_size=CHUNK_SIZE,
        cfg_freq=FREQ,
	cfg_datasize=DEF_DATASIZE,
        cfg_stream_cache_size_chunks=STREAM_CACHE_SIZE_CHUNKS;

int_pair server_data_times_pair=(int_pair){SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};
int_pair client_data_times_pair=(int_pair){CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC};
int_pair server_con_times_pair=(int_pair){SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};
int_pair client_con_times_pair=(int_pair){CLIENT_TIMEOUT_CON_SEC,CLIENT_TIMEOUT_CON_USEC};
int_pair server_drop_chunks_times_pair=(int_pair){SERVER_DROP_CHUNK_TIMEOUT_SEC,SERVER_DROP_CHUNK_TIMEOUT_USEC};


static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void sigint_handler(int useless){

	printf("Saimos no leitor de cfg. Erro: %s\n",strerror(errno));
	exit(useless);
}

void read_values_cfg(void){
	
	signal(SIGINT,sigint_handler);

	if(!(cfg_fp=fopen(CONFIG_FILE_PATH,"r"))){

		raise(SIGINT);
	}
	
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
	sscanf(curr_line_buff,"chunk_size: %lu",&cfg_chunk_size);
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
	sscanf(curr_line_buff,"datasize: %lu",&cfg_datasize);

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
	sscanf(curr_line_buff,"client_timeouts_con: %lu %lu",&client_con_times_pair[0],&client_con_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_timeouts_data: %lu %lu",&server_data_times_pair[0],&server_data_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_timeouts_con: %lu %lu",&server_con_times_pair[0],&server_con_times_pair[1]);

	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_drop_chunk_timeouts: %lu %lu",&server_drop_chunks_times_pair[0],&server_drop_chunks_times_pair[1]);
	clean_buff();

	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_music_folder_path: %s",server_music_folder_path);
	clean_buff();

	fclose(cfg_fp);



}

void print_values_cfg(void){


	printf("chunk_size: %lu\n",cfg_chunk_size/CHANNELS);

	printf("freq: %lu\n",cfg_freq);

	printf("datasize: %lu\n",cfg_datasize);

	printf("client_timeouts_data: %lus %lu us\n",client_data_times_pair[0],client_data_times_pair[1]);

	printf("client_timeouts_con: %lus %lu us\n",client_con_times_pair[0],client_con_times_pair[1]);

	printf("server_timeouts_data: %lus %lu us\n",server_data_times_pair[0],server_data_times_pair[1]);

	printf("server_timeouts_con: %lus %lu us\n",server_con_times_pair[0],server_con_times_pair[1]);


	printf("server_drop_chunk_timeouts: %lus %luus\n",server_drop_chunks_times_pair[0],server_drop_chunks_times_pair[1]);

	printf("stream_chunk_num: %lu\n",cfg_stream_cache_size_chunks);

	printf("server_music_folder_path: %s\n",server_music_folder_path);


}
