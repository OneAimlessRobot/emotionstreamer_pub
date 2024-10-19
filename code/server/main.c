#include "../Includes/preprocessor.h"
#include "Includes/connection.h"
#include "Includes/engine.h"
#include "../extra_funcs/Includes/fileshit.h"

int main(int argc, char ** argv){


        if(argc!=3){

                printf("Precisas do address deste server (arg1) e de uma porta udp(arg2) para o server!!!\n");
                exit(-1);
        }
	memset(curr_dir,0,PATHSIZE);
	getcwd(curr_dir,PATHSIZE-1);
	snprintf(curr_dir+strlen(curr_dir),DEF_DATASIZE,"%s",MUSIC_SERVER_INPUT_PATH);

	printf("Curr dir: %s\n", curr_dir);
	
	serverInit(argv[1],atol(argv[2]));
        //bind the socket to the specified IP and port

       return 0;
}
