#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/auxfuncs.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/engine.h"
#include "Includes/connection.h"
#include "Includes/configs.h"
typedef enum{WORKER=0,SERVER}process_type;
static int this_process_type=0;
int main(int argc, char* argv[]){

	printf("We got %d args at startup!\n",argc);
	if(argc>=5 || argc <= 0){
		printf("Wrong number of args! Exiting...\n");
		return -1;

	}
	this_process_type=((argc==1)?SERVER:atoi(argv[1]));
	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE);
	parse_generalized_cfg();
        read_values_cfg_server();
        printf("Curr dir: %s\n", curr_dir);
	int sockd=-1,
		result=0;
	switch(this_process_type){

		case SERVER:
			if(cfg_server_show_splash){
				print_out_logo();
			}
			if(cfg_server_print_config){
				print_values_generalized_cfg(1);
				print_values_cfg_server(1);
			}
			result=serverInit(&server_ip_cache_entry,&upper_ip_cache_entry);

		       if(result){

				printf("Saimos do server!!!!!!\n");

			}
		break;
		case WORKER:
			if(argc<=2){
				printf("Filedescriptor was not given! Exiting...\n");
				return 1;
			}
			else if(argc<=3){
				printf("request_id was not given! Exiting...\n");
				return 1;
			}
			else{
				sockd = atoi(argv[2]);
				curr_request_id = strtoull(argv[3], NULL, 16);
				printf("Filedescriptor was given! The number is: %d\n",sockd);
				printf("Request id was given! The id is: %lx\n",curr_request_id);
			}
			memset(curr_server_quarantine_dir_buff,0,PATHSIZE+1);
			getcwd(curr_server_quarantine_dir_buff,PATHSIZE);
			result= strnlen(server_music_folder_path,PATHSIZE);
			if(!result){

				snprintf(curr_dir+strlen(curr_dir),PATHSIZE+1,"%s",MUSIC_SERVER_INPUT_PATH);

			}
			else{

				snprintf(curr_dir+strlen(curr_dir),PATHSIZE+1,"%s",server_music_folder_path);

			}
			result= strnlen(server_music_quarantine_folder_path,PATHSIZE);
			if(!result){

				snprintf(curr_server_quarantine_dir_buff+strlen(curr_server_quarantine_dir_buff),PATHSIZE+1,"%s",MUSIC_SERVER_QUARANTINE_PATH);

			}
			else{

				snprintf(curr_server_quarantine_dir_buff+strlen(curr_server_quarantine_dir_buff),PATHSIZE+1,"%s",server_music_quarantine_folder_path);

			}
			con_go(sockd);


		break;
	}
	return 0;
}
