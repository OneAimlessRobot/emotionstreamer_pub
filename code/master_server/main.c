#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/auxfuncs.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/configs.h"
#include "Includes/master_server.h"



int main(void){

	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        parse_generalized_cfg();
        read_values_cfg_master();
        if(cfg_master_show_splash){
		print_out_logo();
	}
        if(cfg_master_print_config){

		print_values_generalized_cfg(1);
		print_values_cfg_master(1);
	}
	printf("Curr dir: %s\n", curr_dir);
	start_master(master_ip_cache_entry.hostname,master_ip_cache_entry.port);

	printf("Saimos do server!!!!!!\n");

	printf("Hello world!!!!\n");
	return 0;

}
