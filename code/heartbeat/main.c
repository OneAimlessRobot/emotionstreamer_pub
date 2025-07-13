#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "Includes/configs.h"
#include "Includes/heart_beat.h"



int main(int argc, char** argv){


	
        read_values_cfg_hb();
        print_values_cfg_hb(1);

        if(argc!=3){

                printf("Precisas do address deste heartbeat server: <ip>:<port>\nE address do gajo de cima: <ip>:<port>\n");
                exit(-1);
        }
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);

        printf("Curr dir: %s\n", curr_dir);
	ip_cache_entry ent={{0},0};
        parse_ip_cache_entry(argv[1],&ent);
        ip_cache_entry ent2={{0},0};
        parse_ip_cache_entry(argv[2],&ent2);

	start_heart_beats(&ent,&ent2);

	printf("Saimos do server!!!!!!\n");

	printf("Hello world!!!!\n");
	
	return 0;

}
