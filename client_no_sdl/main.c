#include "../Includes/preprocessor.h"
#include "Includes/client.h"
#include "../extra_funcs/Includes/fileshit.h"

int main(int argc, char ** argv){

	
	
	if(argc!=5){

		printf("Utilizacao correta: arg1: tipo de pedido (Get ou peek. Obter uma musica ou consultar musicas. Sai logo e a musica fornecida é ignorada).\n arg2: porta do server.\narg3: ipv4 do server\narg4: Nome da musica a tocar\n");
		exit(-1);
	}
	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);

 	snprintf(curr_dir+strlen(curr_dir),BUFFSIZE,"%s",MUSIC_CLIENT_OUTPUT_PATH);

        printf("Curr dir: %s\n", curr_dir);
	clientStart(argv[1],argv[2],atoi(argv[3]),argv[4]);



	return 0;
}
