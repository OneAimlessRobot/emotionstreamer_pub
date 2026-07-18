#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/generalized_config.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
uint8_t cfg_port_mapper_logging=0;
char port_mapper_filepath[PATHSIZE+1]= {0};

uint8_t cfg_port_mapper_print_config,
	cfg_port_mapper_show_splash,
	cfg_port_mapper_use_port_mapper_file,
	cfg_port_mapper_remove_port_file_on_exit;


uint16_t cfg_init_port=10000;
uint16_t cfg_num_ports=1;

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}



static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	printf("Saimos no leitor de cfg. do port_mapper. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_PORT_MAPPER);
	exit(-1);
}






void read_values_cfg_port_mapper(void){


        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_PORT_MAPPER,"r"))){

                clean_and_exit();
        }
        clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"port_mapper_print_config: %hhu",&cfg_port_mapper_print_config);
	clean_buff();
	skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		clean_and_exit();
	}
	sscanf(curr_line_buff,"port_mapper_show_splash: %hhu",&cfg_port_mapper_show_splash);
	clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_logging: %hhu",&cfg_port_mapper_logging);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_init_port: %hu",&cfg_init_port);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_num_ports: %hu",&cfg_num_ports);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_use_port_mapper_file: %hhu",&cfg_port_mapper_use_port_mapper_file);
        clean_buff();
	skip_config_comments(cfg_fp);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_remove_port_file_on_exit: %hhu",&cfg_port_mapper_remove_port_file_on_exit);
        clean_buff();
	fclose(cfg_fp);
}
void print_values_cfg_port_mapper(int fd){


	dprintf(fd,"port_mapper_print_config: %hhu\n",cfg_port_mapper_print_config);

	dprintf(fd,"port_mapper_show_splash: %hhu\n",cfg_port_mapper_show_splash);

	dprintf(fd,"port_mapper_logging: %hhu\n",cfg_port_mapper_logging);

	dprintf(fd,"port_mapper_init_port: %hu\n",cfg_init_port);

	dprintf(fd,"port_mapper_num_ports: %hu\n",cfg_num_ports);

	dprintf(fd,"port_mapper_use_port_mapper_file: %hhu\n",cfg_port_mapper_use_port_mapper_file);

	dprintf(fd,"port_mapper_remove_port_file_on_exit: %hhu\n",cfg_port_mapper_remove_port_file_on_exit);

}
