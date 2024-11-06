#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/heart_beat.h"






static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

//EM BYTES E HZ!

int_pair hb_data_times_pair=(int_pair){HB_TIMEOUT_DATA_SEC,HB_TIMEOUT_DATA_USEC};
int_pair hb_con_times_pair=(int_pair){HB_TIMEOUT_CON_SEC,HB_TIMEOUT_CON_USEC};

uint16_t hb_ack_timeout_lim=HB_ACK_TIMEOUT_LIM;

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
        sscanf(curr_line_buff,"hb_ack_timeout_lim: %hu",&hb_ack_timeout_lim);
        clean_buff();

        fclose(cfg_fp);



}


void print_values_cfg_hb(int fd){

        
        dprintf(fd,"hb_timeouts_con: %lus %lu us\n",hb_con_times_pair[0],hb_con_times_pair[1]);

        dprintf(fd,"hb_timeouts_data: %lus %lu us\n",hb_data_times_pair[0],hb_data_times_pair[1]);

        dprintf(fd,"hb_ack_timeout_lim: %hu\n",hb_ack_timeout_lim);




}
