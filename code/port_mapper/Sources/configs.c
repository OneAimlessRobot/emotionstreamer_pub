#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/generalized_config.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
uint8_t cfg_port_mapper_logging=0;
char port_mapper_filepath[PATHSIZE+1]= {0};
const uint8_t port_mapper_display_splash=0;

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
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_logging: %hhu",&cfg_port_mapper_logging);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_init_port: %hu",&cfg_init_port);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){


                clean_and_exit();
        }
        sscanf(curr_line_buff,"port_mapper_num_ports: %hu",&cfg_num_ports);
        clean_buff();
        fclose(cfg_fp);
}
void print_values_cfg_port_mapper(int fd){


	dprintf(fd,"port_mapper_logging: %hhu\n",cfg_port_mapper_logging);

	dprintf(fd,"port_mapper_init_port: %hu\n",cfg_init_port);

	dprintf(fd,"port_mapper_num_ports: %hu\n",cfg_num_ports);

}
