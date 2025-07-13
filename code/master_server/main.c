#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "Includes/configs.h"
#include "Includes/master_server.h"



int main(int argc, char** argv){


	
        read_values_cfg_master();
        print_values_cfg_master(1);

        if(argc!=2){

                printf("Precisas do address deste master server: <ip>:<port> \n");
                exit(-1);
        }
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);

        printf("Curr dir: %s\n", curr_dir);
        ip_cache_entry ent={{0},0};
        parse_ip_cache_entry(argv[1],&ent);

	start_master(ent.hostname,ent.port);

	printf("Saimos do server!!!!!!\n");

	printf("Hello world!!!!\n");
	
	return 0;

}
