#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/auxfuncs.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/configs.h"
#include "Includes/heart_beat.h"



int main(void){


	print_out_logo();
	read_values_cfg_hb();
        print_values_cfg_hb(1);
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        printf("Generalized cfg:\n");
        parse_generalized_cfg(generalized_config_filepath_buff);
        print_values_generalized_cfg(1);

        printf("Curr dir: %s\n", curr_dir);

	start_heart_beats(&heartbeat_ip_cache_entry,&upper_ip_cache_entry);

	printf("Saimos do server!!!!!!\n");

	printf("Hello world!!!!\n");
	return 0;

}
