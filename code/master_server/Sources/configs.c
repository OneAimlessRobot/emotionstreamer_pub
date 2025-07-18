#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/master_server.h"






static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
char generalized_config_filepath_buff[PATHSIZE+1]={0};
static char master_ip_address_buff[PATHSIZE+1]={0};
static char master_server_port_mapper_ip_address_buff[PATHSIZE+1]={0};

ip_cache_entry master_ip_cache_entry={{0},0};
ip_cache_entry master_server_port_mapper_ip_cache_entry={{0},0};

//EM BYTES E HZ!

int_pair master_data_times_pair=(int_pair){MASTER_TIMEOUT_DATA_SEC,MASTER_TIMEOUT_DATA_USEC};
int_pair master_con_times_pair=(int_pair){MASTER_TIMEOUT_CON_SEC,MASTER_TIMEOUT_CON_USEC};
int_pair master_holepunching_times_pair=(int_pair){HOLE_PUNCHING_TIMEOUT_SEC,HOLE_PUNCHING_TIMEOUT_USEC};

uint16_t master_ack_timeout_lim=MASTER_ACK_TIMEOUT_LIM;

static void process_ip_cache_entries(void){

        parse_ip_cache_entry(master_ip_address_buff,&master_ip_cache_entry);
        parse_ip_cache_entry(master_server_port_mapper_ip_address_buff,&master_server_port_mapper_ip_cache_entry);
}


static void sigint_handler(int useless){

        printf("Saimos no leitor de cfg. do heartbeat server Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_MASTER);
        exit(useless);
}

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

void read_values_cfg_master(void){

        signal(SIGINT,sigint_handler);

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_MASTER,"r"))){

                raise(SIGINT);
        }
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"master_timeouts_con: %lu %lu",&master_con_times_pair[0],&master_con_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"master_timeouts_data: %lu %lu",&master_data_times_pair[0],&master_data_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"master_timeouts_holepunching: %lu %lu",&master_holepunching_times_pair[0],&master_holepunching_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"master_ack_timeout_lim: %hu",&master_ack_timeout_lim);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
	//master_server_port_mapper_ip_address:
        sscanf(curr_line_buff,"master_server_ip_address: %s",master_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"master_server_port_mapper_ip_address: %s",master_server_port_mapper_ip_address_buff);
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


void print_values_cfg_master(int fd){

        
        dprintf(fd,"master_timeouts_con: %lus %lu us\n",master_con_times_pair[0],master_con_times_pair[1]);

        dprintf(fd,"master_timeouts_data: %lus %lu us\n",master_data_times_pair[0],master_data_times_pair[1]);

        dprintf(fd,"master_timeouts_holepunching: %lus %lu us\n",master_holepunching_times_pair[0],master_holepunching_times_pair[1]);

        dprintf(fd,"generalized_config_filepath: %s\n",generalized_config_filepath_buff);

	print_ip_cache_entry(stdout,&master_ip_cache_entry);

	print_ip_cache_entry(stdout,&master_server_port_mapper_ip_cache_entry);


}
