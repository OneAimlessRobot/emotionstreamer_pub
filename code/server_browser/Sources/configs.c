#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"





static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

//EM BYTES E HZ!

int_pair browser_data_times_pair=(int_pair){BROWSER_TIMEOUT_DATA_SEC,BROWSER_TIMEOUT_DATA_USEC};
int_pair browser_con_times_pair=(int_pair){BROWSER_TIMEOUT_CON_SEC,BROWSER_TIMEOUT_CON_USEC};

static void sigint_handler(int useless){

        printf("Saimos no leitor de cfg. do heartbeat server Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_BROWSER);
        exit(useless);
}

static void clean_buff(void){

        memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

void read_values_cfg_browser(void){

        signal(SIGINT,sigint_handler);

        if(!(cfg_fp=fopen(CONFIG_FILE_PATH_BROWSER,"r"))){

                raise(SIGINT);
        }
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"browser_timeouts_con: %lu %lu",&browser_con_times_pair[0],&browser_con_times_pair[1]);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                fclose(cfg_fp);
                raise(SIGINT);
        }
        sscanf(curr_line_buff,"browser_timeouts_data: %lu %lu",&browser_data_times_pair[0],&browser_data_times_pair[1]);
        clean_buff();

        fclose(cfg_fp);



}


void print_values_cfg_browser(int fd){


        dprintf(fd,"browser_timeouts_con: %lus %lu us\n",browser_con_times_pair[0],browser_con_times_pair[1]);

        dprintf(fd,"browser_timeouts_data: %lus %lu us\n",browser_data_times_pair[0],browser_data_times_pair[1]);




}
