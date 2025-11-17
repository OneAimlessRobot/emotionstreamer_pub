#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/heart_beat.h"






static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

static char heartbeat_ip_address_buff[PATHSIZE+1]={0};
static char heartbeat_port_mapper_ip_address_buff[PATHSIZE+1]={0};
static char upper_ip_address_buff[PATHSIZE+1]={0};

char generalized_config_filepath_buff[PATHSIZE+1]={0};
char hb_server_name_buff[PATHSIZE+1]={0};
int8_t hb_heartbeat_protocol;

ip_cache_entry heartbeat_ip_cache_entry={{0},0};
ip_cache_entry upper_ip_cache_entry={{0},0};
ip_cache_entry heartbeat_port_mapper_ip_entry={{0},0};

//EM BYTES E HZ!

int_pair hb_data_times_pair=(int_pair){HB_TIMEOUT_DATA_SEC,HB_TIMEOUT_DATA_USEC};
int_pair hb_con_times_pair=(int_pair){HB_TIMEOUT_CON_SEC,HB_TIMEOUT_CON_USEC};
int_pair hb_ack_times_pair=(int_pair){HB_TIMEOUT_ACK_SEC,HB_TIMEOUT_ACK_USEC};

uint64_t cfg_hb_ack_period_us=DEF_HB_ACK_PERIOD_US;
uint8_t cfg_hb_server_logging=0;

static void process_ip_cache_entries(void){

	parse_ip_cache_entry(heartbeat_ip_address_buff,&heartbeat_ip_cache_entry);
	parse_ip_cache_entry(upper_ip_address_buff,&upper_ip_cache_entry);
	parse_ip_cache_entry(heartbeat_port_mapper_ip_address_buff,&heartbeat_port_mapper_ip_entry);

}

static void sigint_handler(int useless){

        printf("Saimos no leitor de cfg. do heartbeat server Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_HB);
        exit(useless);
}

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

void read_values_cfg_hb(void){

        signal(SIGINT,sigint_handler);

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_HB,"r"))){

                raise(SIGINT);
        }
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"hb_server_logging: %hhu",&cfg_hb_server_logging);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"hb_timeouts_con: %lu %lu",&hb_con_times_pair[0],&hb_con_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"hb_timeouts_data: %lu %lu",&hb_data_times_pair[0],&hb_data_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"hb_timeouts_ack: %lu %lu",&hb_ack_times_pair[0],&hb_ack_times_pair[1]);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"hb_ack_period_us: %lu",&cfg_hb_ack_period_us);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"heartbeat_ip_address: %s",heartbeat_ip_address_buff);
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
        sscanf(curr_line_buff,"heartbeat_port_mapper_ip_address: %s",heartbeat_port_mapper_ip_address_buff);
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
        sscanf(curr_line_buff,"hb_server_name: %s",hb_server_name_buff);
        clean_buff();
        fclose(cfg_fp);

	process_ip_cache_entries();

}

void print_values_cfg_hb(int fd){

        dprintf(fd,"hb_server_logging: %hhu\n",cfg_hb_server_logging);

        dprintf(fd,"hb_timeouts_con: %lus %lu us\n",hb_con_times_pair[0],hb_con_times_pair[1]);

        dprintf(fd,"hb_timeouts_data: %lus %lu us\n",hb_data_times_pair[0],hb_data_times_pair[1]);

        dprintf(fd,"hb_timeouts_ack: %lus %lu us\n",hb_ack_times_pair[0],hb_ack_times_pair[1]);

	dprintf(fd,"hb_ack_period_us: %luus\n",cfg_hb_ack_period_us);

        dprintf(fd,"generalized_config_filepath: %s\n",generalized_config_filepath_buff);

	dprintf(fd,"hb_server_name: %s\n",hb_server_name_buff);

	print_ip_cache_entry(stdout,&heartbeat_ip_cache_entry);

	print_ip_cache_entry(stdout,&upper_ip_cache_entry);

	print_ip_cache_entry(stdout,&heartbeat_port_mapper_ip_entry);

	
}
