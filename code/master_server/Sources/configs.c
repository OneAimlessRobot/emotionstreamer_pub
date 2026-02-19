#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/master_server.h"
#include "../../extra_funcs/Includes/generalized_config.h"






static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

uint8_t	cfg_master_print_config,
	cfg_master_show_splash;

ip_cache_entry master_ip_cache_entry = {{0},0};

int_pair master_data_times_pair=(int_pair){MASTER_TIMEOUT_DATA_SEC,MASTER_TIMEOUT_DATA_USEC};
int_pair master_con_times_pair=(int_pair){MASTER_TIMEOUT_CON_SEC,MASTER_TIMEOUT_CON_USEC};
int_pair master_ack_times_pair=(int_pair){MASTER_TIMEOUT_ACK_SEC,MASTER_TIMEOUT_ACK_USEC};

uint8_t cfg_master_server_logging=0;

uint64_t cfg_master_ack_period_us=DEF_MASTER_ACK_PERIOD_US;

static void process_ip_cache_entries(void){

        parse_ip_cache_entry(port_mapper_ip_address_buff,&master_ip_cache_entry);
}


static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	printf("Saimos no leitor de cfg. do master server. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_MASTER);
	exit(-1);
}

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

void read_values_cfg_master(void){

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_MASTER,"r"))){

                clean_and_exit();
        }
        clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"master_print_config: %hhu",&cfg_master_print_config);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"master_show_splash: %hhu",&cfg_master_show_splash);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"master_server_logging: %hhu",&cfg_master_server_logging);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"master_timeouts_con: %lu %lu",&master_con_times_pair[0],&master_con_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"master_timeouts_data: %lu %lu",&master_data_times_pair[0],&master_data_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"master_timeouts_ack: %lu %lu",&master_ack_times_pair[0],&master_ack_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"master_ack_period_us: %lu",&cfg_master_ack_period_us);
        clean_buff();
        fclose(cfg_fp);

	process_ip_cache_entries();



}


void print_values_cfg_master(int fd){

	dprintf(fd,"master_print_config: %hhu\n",cfg_master_print_config);

	dprintf(fd,"master_show_splash: %hhu\n",cfg_master_show_splash);

        dprintf(fd,"master_server_logging: %hhu\n",cfg_master_server_logging);

        dprintf(fd,"master_timeouts_con: %lus %lu us\n",master_con_times_pair[0],master_con_times_pair[1]);

        dprintf(fd,"master_timeouts_data: %lus %lu us\n",master_data_times_pair[0],master_data_times_pair[1]);

        dprintf(fd,"master_timeouts_ack: %lus %lu us\n",master_ack_times_pair[0],master_ack_times_pair[1]);

	dprintf(fd,"master_ack_period_us: %luus\n",cfg_master_ack_period_us);

}
