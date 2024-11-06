#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static int tmp_cfg_fd=-1;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

char server_music_folder_path[PATHSIZE]={0};

//EM BYTES E HZ!

int_pair server_data_times_pair=(int_pair){SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};
int_pair server_con_times_pair=(int_pair){SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};
int_pair server_drop_chunks_times_pair=(int_pair){SERVER_DROP_CHUNK_TIMEOUT_SEC,SERVER_DROP_CHUNK_TIMEOUT_USEC};

uint16_t server_ack_timeout_lim=SERVER_ACK_TIMEOUT_LIM;

uint16_t server_chunk_size=SERVER_CHUNK_SIZE;

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
	sscanf(curr_line_buff,"server_chunk_size: %hu",&server_chunk_size);

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
	sscanf(curr_line_buff,"server_timeouts_data: %lu %lu",&server_data_times_pair[0],&server_data_times_pair[1]);
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
        sscanf(curr_line_buff,"server_ack_timeout_lim: %hu",&server_ack_timeout_lim);
        clean_buff();

	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_music_folder_path: %s",server_music_folder_path);
	clean_buff();

	fclose(cfg_fp);



}


void produce_config_file(void){
	tmp_cfg_fd=open(TMP_CONFIG_FILE_PATH,O_WRONLY|O_CREAT|O_TRUNC,0777);
	if(tmp_cfg_fd<0){
		perror("Nao foi possivel abrir ficheiro de configs do server");
		return;
	}
	print_values_cfg_server(tmp_cfg_fd);
	close(tmp_cfg_fd);

}

void print_values_cfg_server(int fd){

	dprintf(fd,"server_chunk_size: %hu\n",server_chunk_size);

	dprintf(fd,"server_timeouts_data: %lus %lu us\n",server_data_times_pair[0],server_data_times_pair[1]);

	dprintf(fd,"server_timeouts_con: %lus %lu us\n",server_con_times_pair[0],server_con_times_pair[1]);

	dprintf(fd,"server_timeouts_drop_chunks: %lus %lu us\n",server_drop_chunks_times_pair[0],server_drop_chunks_times_pair[1]);

	dprintf(fd,"server_ack_timeout_lim: %hu\n",server_ack_timeout_lim);




}
