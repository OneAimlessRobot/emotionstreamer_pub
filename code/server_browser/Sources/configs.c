#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/generalized_config.h"





static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};


ip_cache_entry server_browser_ip_cache_entry={{0},0};


int_pair browser_data_times_pair=(int_pair){BROWSER_TIMEOUT_DATA_SEC,BROWSER_TIMEOUT_DATA_USEC};
int_pair browser_con_times_pair=(int_pair){BROWSER_TIMEOUT_CON_SEC,BROWSER_TIMEOUT_CON_USEC};

uint8_t	cfg_browser_print_config,
	cfg_browser_show_splash;

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

        parse_ip_cache_entry(port_mapper_ip_address_buff,&server_browser_ip_cache_entry);
}

void read_values_cfg_browser(void){

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_BROWSER,"r"))){

                clean_and_exit();
        }
        clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"browser_print_config: %hhu",&cfg_browser_print_config);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"browser_show_splash: %hhu",&cfg_browser_show_splash);
	clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_logging: %hhu",&cfg_server_browser_logging);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_timeouts_con: %lu %lu",&browser_con_times_pair[0],&browser_con_times_pair[1]);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_timeouts_data: %lu %lu",&browser_data_times_pair[0],&browser_data_times_pair[1]);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"browser_using_tls: %hu",&will_use_tls);
        clean_buff();
	skip_config_comments(cfg_fp);
	if(will_use_tls){

		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	                clean_and_exit();
	        }
	        sscanf(curr_line_buff,"browser_auth_cert_file_path: %s",auth_cert_file_path);
	        clean_buff();
		skip_config_comments(cfg_fp);
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	                clean_and_exit();
	        }
	        sscanf(curr_line_buff,"browser_host_cert_file_path: %s",host_cert_file_path);
	        clean_buff();
		skip_config_comments(cfg_fp);
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	                clean_and_exit();
	        }
	        sscanf(curr_line_buff,"browser_host_pkey_file_path: %s",host_pkey_file_path);
	        clean_buff();
	}
	fclose(cfg_fp);
	auth_cert_file_path[sizeof(auth_cert_file_path)-1]=0;
	host_cert_file_path[sizeof(host_cert_file_path)-1]=0;
	host_pkey_file_path[sizeof(host_pkey_file_path)-1]=0;
	process_ip_cache_entries();


}


void print_values_cfg_browser(int fd){

	dprintf(fd,"browser_print_config: %hhu\n",cfg_browser_print_config);

	dprintf(fd,"browser_show_splash: %hhu\n",cfg_browser_show_splash);

        dprintf(fd,"browser_logging: %hhu\n",cfg_server_browser_logging);

        dprintf(fd,"browser_timeouts_con: %lus %lu us\n",browser_con_times_pair[0],browser_con_times_pair[1]);

        dprintf(fd,"browser_timeouts_data: %lus %lu us\n",browser_data_times_pair[0],browser_data_times_pair[1]);

	dprintf(fd,"browser_using_tls: %hu\n",will_use_tls);

	if(will_use_tls){
		dprintf(fd,"browser_auth_cert_file_path: %s\n",auth_cert_file_path);

		dprintf(fd,"browser_host_cert_file_path: %s\n",host_cert_file_path);

		dprintf(fd,"browser_host_pkey_file_path: %s\n",host_pkey_file_path);
	}
}
