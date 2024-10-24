#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

char server_music_folder_path[PATHSIZE]={0};

//EM BYTES E HZ!

int_pair server_data_times_pair=(int_pair){SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};
int_pair server_con_times_pair=(int_pair){SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};
int_pair server_drop_chunks_times_pair=(int_pair){SERVER_DROP_CHUNK_TIMEOUT_SEC,SERVER_DROP_CHUNK_TIMEOUT_USEC};


static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void sigint_handler(int useless){

	printf("Saimos no leitor de cfg. do server Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_SERVER);
	exit(useless);
}

void read_values_cfg_server(void){
	
	signal(SIGINT,sigint_handler);

	if(!(cfg_fp=fopen(CONFIG_FILE_PATH_SERVER,"r"))){

		raise(SIGINT);
	}
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
	sscanf(curr_line_buff,"server_timeouts_drop_chunks: %lu %lu",&server_drop_chunks_times_pair[0],&server_drop_chunks_times_pair[1]);
	clean_buff();

	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_music_folder_path: %s",server_music_folder_path);
	clean_buff();

	fclose(cfg_fp);



}

void print_values_cfg_server(void){

	printf("server_timeouts_data: %lus %lu us\n",server_data_times_pair[0],server_data_times_pair[1]);

	printf("server_timeouts_con: %lus %lu us\n",server_con_times_pair[0],server_con_times_pair[1]);

	printf("server_timeouts_drop_chunks: %lus %lu us\n",server_drop_chunks_times_pair[0],server_drop_chunks_times_pair[1]);

	printf("server_music_folder_path: %s\n",server_music_folder_path);


}
