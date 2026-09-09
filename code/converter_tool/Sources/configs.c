#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/converter.h"

static FILE* cfg_fp=NULL;

static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

char converter_in_dir[PATHSIZE]={0};

char converter_out_dir[PATHSIZE]={0};

const uint8_t converter_tool_display_splash=0;




static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}


static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	printf("Saimos no leitor de cfg. do client. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_CONVERTER);
	exit(-1);
}

void read_values_cfg_converter(void){


        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_CONVERTER,"r"))){

		clean_and_exit();
        }
        clean_buff();
        skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();

        }
        curr_line_buff[strlen(curr_line_buff)-1]=0;
        snprintf(converter_in_dir,sizeof(converter_in_dir),"%s",(char*)&curr_line_buff[strlen("input_dir: ")]);
	clean_buff();
        skip_config_comments(cfg_fp);
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();

        }
        curr_line_buff[strlen(curr_line_buff)-1]=0;
        snprintf(converter_out_dir,sizeof(converter_out_dir),"%s",(char*)&curr_line_buff[strlen("output_dir: ")]);
	clean_buff();
 	fclose(cfg_fp);
}


void print_values_cfg_converter(int fd){

	dprintf(fd,"input_dir: %s\n",converter_in_dir);

	dprintf(fd,"output_dir: %s\n",converter_out_dir);

}
