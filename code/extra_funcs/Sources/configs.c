#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/streamer_const.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};
static char client_logs_file_name[PATHSIZE]={0};
//EM BYTES E HZ!
u_int64_t cfg_chunk_size=CHUNK_SIZE,
	cfg_datasize=DEF_DATASIZE;


static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void sigint_handler(int useless){

	printf("Saimos no leitor de cfg. geral! Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH);
	exit(useless);
}

void read_values_cfg_general(void){
	
	signal(SIGINT,sigint_handler);

	if(!(cfg_fp=fopen(CONFIG_FILE_PATH,"r"))){

		raise(SIGINT);
	}
	
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"datasize: %lu",&cfg_datasize);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

		fclose(cfg_fp);
		raise(SIGINT);
	}
	sscanf(curr_line_buff,"chunk_size: %lu",&cfg_chunk_size);
	clean_buff();

	fclose(cfg_fp);



}

void print_values_cfg_general(void){


	printf("chunk_size: %lu bytes\n",cfg_chunk_size);

	printf("datasize: %lu bytes\n",cfg_datasize);


}
