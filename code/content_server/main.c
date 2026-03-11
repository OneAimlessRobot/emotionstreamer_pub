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

int main(void){

	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE);
	parse_generalized_cfg();
        read_values_cfg_server();
        if(cfg_server_show_splash){
		print_out_logo();
	}
	if(cfg_server_print_config){
		print_values_generalized_cfg(1);
		print_values_cfg_server(1);
	}
	memset(curr_server_quarantine_dir_buff,0,PATHSIZE+1);
	getcwd(curr_server_quarantine_dir_buff,PATHSIZE);
	int result= strnlen(server_music_folder_path,PATHSIZE);
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
	printf("Curr dir: %s\n", curr_dir);
	result=serverInit(&server_ip_cache_entry,&upper_ip_cache_entry);

       if(result){

		printf("Saimos do server!!!!!!\n");
     
	}
	return 0;
}
