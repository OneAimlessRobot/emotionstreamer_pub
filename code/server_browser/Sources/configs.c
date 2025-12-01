#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"





static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};


static char server_browser_port_mapper_ip_buff[PATHSIZE]={0};
static char server_browser_ip_buff[PATHSIZE]={0};
//EM BYTES E HZ!
ip_cache_entry server_browser_port_mapper_ip_cache_entry={{0},0};
ip_cache_entry server_browser_ip_cache_entry={{0},0};


int_pair browser_data_times_pair=(int_pair){BROWSER_TIMEOUT_DATA_SEC,BROWSER_TIMEOUT_DATA_USEC};
int_pair browser_con_times_pair=(int_pair){BROWSER_TIMEOUT_CON_SEC,BROWSER_TIMEOUT_CON_USEC};

const uint8_t browser_display_splash=0;

uint8_t cfg_server_browser_logging=0;

uint64_t cfg_browser_ack_period_us=DEF_BROWSER_ACK_PERIOD_US;

char generalized_config_filepath_buff[PATHSIZE+1]={0};

static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	printf("Saimos no leitor de cfg. do server_browser. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_BROWSER);
	exit(-1);
}

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void process_ip_cache_entries(void){

        parse_ip_cache_entry(server_browser_ip_buff,&server_browser_ip_cache_entry);
        parse_ip_cache_entry(server_browser_port_mapper_ip_buff,&server_browser_port_mapper_ip_cache_entry);
}

void read_values_cfg_browser(void){

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_BROWSER,"r"))){

                clean_and_exit();
        }
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_logging: %hhu",&cfg_server_browser_logging);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_timeouts_con: %lu %lu",&browser_con_times_pair[0],&browser_con_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_timeouts_data: %lu %lu",&browser_data_times_pair[0],&browser_data_times_pair[1]);
        clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_browser_port_mapper_ip_address: %s",server_browser_port_mapper_ip_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_browser_ip_address: %s",server_browser_ip_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"generalized_config_path: %s",generalized_config_filepath_buff);
        clean_buff();
        fclose(cfg_fp);

	process_ip_cache_entries();


}


void print_values_cfg_browser(int fd){


        dprintf(fd,"browser_logging: %hhu\n",cfg_server_browser_logging);

        dprintf(fd,"browser_timeouts_con: %lus %lu us\n",browser_con_times_pair[0],browser_con_times_pair[1]);

        dprintf(fd,"browser_timeouts_data: %lus %lu us\n",browser_data_times_pair[0],browser_data_times_pair[1]);

        dprintf(fd,"generalized_config_path: %s\n",generalized_config_filepath_buff);

	print_ip_cache_entry(stdout,&server_browser_ip_cache_entry);

	print_ip_cache_entry(stdout,&server_browser_port_mapper_ip_cache_entry);



}
