#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static int tmp_cfg_fd=-1;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

static char server_ip_address_buff[PATHSIZE+1]={0};
static char upper_ip_address_buff[PATHSIZE+1]={0};
static char server_port_mapper_ip_address_buff[PATHSIZE+1]={0};
char generalized_config_filepath_buff[PATHSIZE+1]={0};
char server_name_buff[PATHSIZE+1]={0};
ip_cache_entry server_ip_cache_entry={{0},0};
ip_cache_entry upper_ip_cache_entry={{0},0};
ip_cache_entry server_port_mapper_ip_cache_entry={{0},0};

char server_music_folder_path[PATHSIZE+1]={0};
char server_music_quarantine_folder_path[PATHSIZE+1]={0};
char curr_server_quarantine_dir_buff[PATHSIZE+1]={0};

char server_working_extension[EXTENSION_SIZE]={0};

uint8_t cfg_server_logging=0;

//EM BYTES E HZ!

int_pair server_data_times_pair=(int_pair){SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};
int_pair server_con_times_pair=(int_pair){SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};
int_pair server_ack_times_pair=(int_pair){SERVER_TIMEOUT_ACK_SEC,SERVER_TIMEOUT_ACK_USEC};
int_pair server_drop_chunks_times_pair=(int_pair){SERVER_DROP_CHUNK_TIMEOUT_SEC,SERVER_DROP_CHUNK_TIMEOUT_USEC};

uint64_t cfg_server_ack_period_us=DEF_SERVER_ACK_PERIOD_US;


uint64_t server_chunk_size=SERVER_CHUNK_SIZE;
int8_t is_wav_mode=0;
static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void process_ip_cache_entries(void){

        parse_ip_cache_entry(server_ip_address_buff,&server_ip_cache_entry);
        parse_ip_cache_entry(upper_ip_address_buff,&upper_ip_cache_entry);
        parse_ip_cache_entry(server_port_mapper_ip_address_buff,&server_port_mapper_ip_cache_entry);

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
	sscanf(curr_line_buff,"server_logging: %hhu",&cfg_server_logging);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_chunk_size: %lu",&server_chunk_size);
	server_chunk_size=max(0,min(MAX_MP3_STREAM_CHUNK_BUFF_SIZE,server_chunk_size));
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
	sscanf(curr_line_buff,"server_timeouts_ack: %lu %lu",&server_ack_times_pair[0],&server_ack_times_pair[1]);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"server_ack_period_us: %lu",&cfg_server_ack_period_us);
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

	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_music_quarantine_folder_path: %s",server_music_quarantine_folder_path);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"server_working_extension: %s",server_working_extension);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"server_ip_address: %s",server_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"upper_server_ip_address: %s",upper_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"server_port_mapper_ip_address: %s",server_port_mapper_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"generalized_config_filepath: %s",generalized_config_filepath_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"server_name: %s",server_name_buff);
        clean_buff();
        fclose(cfg_fp);
	server_working_extension[sizeof(server_working_extension)-1]=0;
	server_music_folder_path[sizeof(server_music_folder_path)-1]=0;
	
	process_ip_cache_entries();


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

	dprintf(fd,"server_logging: %hhu\n",cfg_server_logging);

	dprintf(fd,"server_chunk_size: %lu (max: %u)\n",server_chunk_size,MAX_MP3_STREAM_CHUNK_BUFF_SIZE);

	dprintf(fd,"server_timeouts_data: %lus %lu us\n",server_data_times_pair[0],server_data_times_pair[1]);

	dprintf(fd,"server_timeouts_con: %lus %lu us\n",server_con_times_pair[0],server_con_times_pair[1]);

	dprintf(fd,"server_timeouts_ack: %lus %lu us\n",server_ack_times_pair[0],server_ack_times_pair[1]);

	dprintf(fd,"server_ack_period_us: %luus\n",cfg_server_ack_period_us);

	dprintf(fd,"server_timeouts_drop_chunks: %lus %lu us\n",server_drop_chunks_times_pair[0],server_drop_chunks_times_pair[1]);

	dprintf(fd,"server_music_folder_path: %s\n",server_music_folder_path);

	dprintf(fd,"server_music_quarantine_folder_path: %s\n",server_music_quarantine_folder_path);

	dprintf(fd,"server_working_extension: %s\n",server_working_extension);

        dprintf(fd,"generalized_config_filepath: %s\n",generalized_config_filepath_buff);

        dprintf(fd,"server_name: %s\n",server_name_buff);

	print_ip_cache_entry(stdout,&server_ip_cache_entry);

	print_ip_cache_entry(stdout,&upper_ip_cache_entry);

	print_ip_cache_entry(stdout,&server_port_mapper_ip_cache_entry);



}
