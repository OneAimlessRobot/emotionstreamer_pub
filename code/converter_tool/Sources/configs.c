#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/converter.h"

static FILE* cfg_fp=NULL;

static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

char converter_in_dir[PATHSIZE]={0};

char converter_out_dir[PATHSIZE]={0};





static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}


static void sigint_handler(int useless){

        printf("Saimos no leitor de cfg. do server Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_CONVERTER);
        exit(useless);
}

void read_values_cfg_converter(void){

        signal(SIGINT,sigint_handler);

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_CONVERTER,"r"))){

                raise(SIGINT);
        }
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"input_dir: %s",converter_in_dir);
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"output_dir: %s",converter_out_dir);
        clean_buff();
 	fclose(cfg_fp);
}


void print_values_cfg_converter(int fd){

	dprintf(fd,"input_dir: %s\n",converter_in_dir);

	dprintf(fd,"output_dir: %s\n",converter_out_dir);

}
