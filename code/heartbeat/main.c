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

	if(hb_display_splash){
		print_out_logo();
	}
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        parse_generalized_cfg();
        print_values_generalized_cfg(1);
	read_values_cfg_hb();
        print_values_cfg_hb(1);
        printf("Curr dir: %s\n", curr_dir);

	start_heart_beats(&heartbeat_ip_cache_entry,&upper_ip_cache_entry);

	printf("Saimos do server!!!!!!\n");

	printf("Hello world!!!!\n");
	return 0;

}
