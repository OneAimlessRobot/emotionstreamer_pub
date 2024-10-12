/*#include "../../player/SDL2/include/SDL.h"
#include "../../player/SDL2/include/SDL_mixer.h"
#include "../Includes/preprocessor.h"
#include "../Includes/menu.h"
#include "../Includes/player.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"


Mix_Music* music=NULL;
static int menu_running=1;
void cleanSDL(void){


	
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

}
void initSDL(void){
	
	if(SDL_Init(SDL_INIT_AUDIO)){
		fprintf(logstream,"Erro a inicializar SDL!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	if(!Mix_Init(MIX_INIT_MP3)){
		fprintf(logstream,"Erro a inicializar SDL_MIXER!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	if(Mix_OpenAudio(FREQ,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,CHUNK_BIG)){
		fprintf(logstream,"Erro a abrir SDL AUDIO!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	



}

void prep_music(const char* filename){

	music= Mix_LoadMUS(filename);
	printf("Sucessfully playing sound!!!!\n");
	Mix_PlayMusic(music,0);
	//Mix_PauseMusic();

}
void delete_music(void){

	Mix_HaltMusic();
	Mix_FreeMusic(music);

}
void cleanup(int useless){
		delete_music();
		cleanSDL();
		exit(useless);

}




void toggle(void){
	
	if(Mix_PausedMusic()){
		
		Mix_ResumeMusic();

	}
	else{
		Mix_PauseMusic();
	}
	
}

int stop(void){

	
	cleanup(-1);
	return 0;
}
void menu_guts(char c){

	switch(c){

		case 'g':
		toggle();
		break;
		case 's':
		stop();
		break;
		default:
		break;

	}


}



void menu_loop(void){

	char input=0;
	do{
		if(!menu_running){

			break;
		}
		print_menu();
		scanf("%c",&input);
		if(receive_cmd(input)){

			menu_guts(input);

		}
		else{

			printf("Input invalida\n");
		}



	}while(input!='s');

	



}
int player_init(const char* filename){
	
	signal(SIGINT,cleanup);
	initSDL();

	prep_music(filename);

	int duration_ms= ((int)(Mix_MusicDuration(music)*1000.0));

	SDL_Delay(duration_ms);

	raise(SIGINT);

	return 0;
}


*/
