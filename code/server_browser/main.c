#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "Includes/configs.h"
#include "Includes/browser.h"

int main(int argc, char ** argv){

        read_values_cfg_browser();
        print_values_cfg_browser(1);
        if(argc!=3){

                printf("Precisas de:1- especificar se estas a browsar...1- master server (master) ou...\n2- heartbeat server\n2- address de server para scannear: <ip>:<port> \n");
                exit(-1);
        }
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        

        printf("Curr dir: %s\n", curr_dir);
        ip_cache_entry ent={{0},0};
        parse_ip_cache_entry(argv[2],&ent);

        init_browser(ent.hostname,argv[1],ent.port);

        printf("Saimos do server browser!!!!!!\n");

        printf("Hello world!!!!\n");
}
