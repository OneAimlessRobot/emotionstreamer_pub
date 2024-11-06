#include "../Includes/preprocessor.h"
#include "Includes/client.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "Includes/configs.h"

int main(int argc, char ** argv){


	read_values_cfg_client();
	print_values_cfg_client(1);
	logging=0;
	int fd=-1;
	int result= strnlen(client_music_folder_path,PATHSIZE-1);
	if(!result){
        	fd= open(LOG_FILE_NAME_CLIENT,O_TRUNC|O_WRONLY|O_CREAT,0777);
	}
	else{
        	fd= open(client_logs_file_name,O_TRUNC|O_WRONLY|O_CREAT,0777);
	}
	if(fd<0){
		perror("Não foi possivel criar ficheiro de logs!\n");
		fd=1;

	}
	logstream=fdopen(fd,"w");
	if(argc!=4){

		printf("Utilizacao correta:\narg1: tipo de pedido (play ou peek. Tocar uma musica ou consultar musicas. Com Peek, Sai logo e a musica fornecida é ignorada).\narg2: Nome da musica a tocar\n arg3: <ip>:<port> do server\n");
		exit(-1);
	}

	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        result= strnlen(client_music_folder_path,PATHSIZE-1);
        if(!result){

                snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",MUSIC_CLIENT_INPUT_PATH);
        }
        else{
                 snprintf(curr_dir+strlen(curr_dir),PATHSIZE,"%s",client_music_folder_path);
	}
	
	printf("Curr dir: %s\n", curr_dir);
	clientStart(argv[1],argv[2],argv[3]);


	return 0;
}
