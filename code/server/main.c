#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/engine.h"
#include "Includes/connection.h"
#include "Includes/configs.h"

int main(void){

	read_values_cfg_server();
        print_values_cfg_server(1);
	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE-1);
	printf("Generalized cfg:\n");
	parse_generalized_cfg(generalized_config_filepath_buff);
	print_values_generalized_cfg(1);
	int result= strnlen(server_music_folder_path,PATHSIZE-1);
	if(!result){

		snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",MUSIC_SERVER_INPUT_PATH);

	}
	else{

		snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",server_music_folder_path);

	}
	
	printf("Curr dir: %s\n", curr_dir);
	result=serverInit(&server_ip_cache_entry,&upper_ip_cache_entry);
	
       if(result){

		printf("Saimos do server!!!!!!\n");
     
	}
	return 0;
}
