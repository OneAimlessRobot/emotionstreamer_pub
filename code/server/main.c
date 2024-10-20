#include "../Includes/preprocessor.h"
#include "Includes/connection.h"
#include "Includes/engine.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/configs.h"

int main(int argc, char ** argv){

	read_values_cfg();
        print_values_cfg();
	if(argc!=3){

                printf("Precisas do address deste server (arg1) e de uma porta udp(arg2) para o server!!!\n");
                exit(-1);
        }
	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE-1);
	int result= strnlen(server_music_folder_path,PATHSIZE-1);
	if(!result){

		snprintf(curr_dir+strlen(curr_dir),cfg_datasize,"%s",MUSIC_SERVER_INPUT_PATH);

	}
	else{

		snprintf(curr_dir+strlen(curr_dir),cfg_datasize,"%s",server_music_folder_path);

	}
	printf("Curr dir: %s\n", curr_dir);

	serverInit(argv[1],atol(argv[2]));
        //bind the socket to the specified IP and port

       return 0;
}
