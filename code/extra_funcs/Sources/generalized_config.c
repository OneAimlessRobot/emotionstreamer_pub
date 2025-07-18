#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/generalized_config.h"
#include "../Includes/fileshit.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

int_pair port_mapper_times_pair={REMAPPER_TIMEOUT_CON_SEC,REMAPPER_TIMEOUT_CON_USEC};

static char cfg_full_file_path[PATHSIZE*3+1]={0};

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}



static void sigint_handler(int useless){

        printf("Saimos no leitor de cfg. do server Erro: %s\nPath para config: %s\n",strerror(errno),cfg_full_file_path);
        exit(useless);
}

void parse_generalized_cfg(char* dir_path){

        signal(SIGINT,sigint_handler);
	snprintf(cfg_full_file_path,PATHSIZE+2,"%s%s%s",curr_dir,dir_path,CONFIG_FILENAME);
        if(!(cfg_fp=fopen(cfg_full_file_path,"r"))){

                raise(SIGINT);
        }
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"port_mapper_timeouts_con: %lu %lu",&port_mapper_times_pair[0],&port_mapper_times_pair[1]);
        clean_buff();
        fclose(cfg_fp);


}

void print_values_generalized_cfg(int fd){

	dprintf(fd,"port_mapper_timeouts_con: %lus %lu us\n",port_mapper_times_pair[0],port_mapper_times_pair[1]);

}
