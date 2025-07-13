#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/master_server.h"






static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

//EM BYTES E HZ!

int_pair master_data_times_pair=(int_pair){MASTER_TIMEOUT_DATA_SEC,MASTER_TIMEOUT_DATA_USEC};
int_pair master_con_times_pair=(int_pair){MASTER_TIMEOUT_CON_SEC,MASTER_TIMEOUT_CON_USEC};

uint16_t master_ack_timeout_lim=MASTER_ACK_TIMEOUT_LIM;

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
        sscanf(curr_line_buff,"master_ack_timeout_lim: %hu",&master_ack_timeout_lim);
        clean_buff();

        fclose(cfg_fp);



}


void print_values_cfg_master(int fd){

        
        dprintf(fd,"master_timeouts_con: %lus %lu us\n",master_con_times_pair[0],master_con_times_pair[1]);

        dprintf(fd,"master_timeouts_data: %lus %lu us\n",master_data_times_pair[0],master_data_times_pair[1]);

        dprintf(fd,"master_ack_timeout_lim: %hu\n",master_ack_timeout_lim);




}
