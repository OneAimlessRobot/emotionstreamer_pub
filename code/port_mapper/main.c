#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/auxfuncs.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/mapper.h"
#include "Includes/port_mapper_file.h"
#include "Includes/configs.h"

int main(void){

	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);

	parse_generalized_cfg();
        read_values_cfg_port_mapper();
        if(cfg_port_mapper_show_splash){
		print_out_logo();
	}
        if(cfg_port_mapper_print_config){
		printf("Generalized cfg:\n");
        	print_values_generalized_cfg(1);
		printf("Normal cfg:\n");
		print_values_cfg_port_mapper(1);
        }
	printf("Curr dir: %s\n", curr_dir);
	port_mapper_init(&port_mapper_ip_cache_entry);

        printf("Saimos do port mapper!!!!!!\n");
        return 0;



	return 0;
}
