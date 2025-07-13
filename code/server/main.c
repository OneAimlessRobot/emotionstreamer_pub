#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "Includes/engine.h"
#include "Includes/connection.h"
#include "Includes/configs.h"

int main(int argc, char ** argv){

	read_values_cfg_server();
        print_values_cfg_server(1);
	if(argc!=3){

		printf("Precisas do address deste heartbeat server: <ip>:<port>\nE address do gajo de cima: <ip>:<port>\n");
                exit(-1);
        }
	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE-1);
	int result= strnlen(server_music_folder_path,PATHSIZE-1);
	if(!result){

		snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",MUSIC_SERVER_INPUT_PATH);

	}
	else{

		snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",server_music_folder_path);

	}
	
	printf("Curr dir: %s\n", curr_dir);
	ip_cache_entry ent={{0},0};
	parse_ip_cache_entry(argv[1],&ent);
	ip_cache_entry ent2={{0},0};
	parse_ip_cache_entry(argv[2],&ent2);
	result=serverInit(&ent,&ent2);


       if(result){

		printf("Saimos do server!!!!!!\n");
     
	}
	return 0;
}
