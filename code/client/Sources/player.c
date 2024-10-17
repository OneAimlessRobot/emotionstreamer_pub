#include "../Includes/preprocessor.h"
/*#include "../../player/SDL2/include/SDL.h"
#include "../../player/SDL2/include/SDL_mixer.h"
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../Includes/menu.h"
#include "../Includes/player.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"


static int duration_ms=-1;
static thread_mtx_cond_struct
		menu_controller ={0, PTHREAD_MUTEX_INITIALIZER,0,PTHREAD_COND_INITIALIZER};


static Mix_Music* music=NULL;
static int menu_running=1;




static void cleanSDL(void){


	
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

}
static void initSDL(void){
	
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
static void prep_music(const char* filename){

	music= Mix_LoadMUS(filename);
	printf("Sucessfully playing sound!!!!\n");
	Mix_PlayMusic(music,0);
	Mix_PauseMusic();
}
static void delete_music(void){

	Mix_HaltMusic();
	Mix_FreeMusic(music);

}


static void toggle(void){
	
	if(Mix_PausedMusic()){
		
		Mix_ResumeMusic();
		printf("PLAYING!!!!!!\n");
	}
	else{
		
		Mix_PauseMusic();
		printf("NOT PLAYING!!!!!!\n");
	}
	
}
static void check_time_left(void){
	int timeleft_ms= ((int)(Mix_MusicDuration(music)*1000.0))-((int)(Mix_GetMusicPosition(music)*1000.0));
	float timeleft_s= (((float)timeleft_ms)/1000.0f);
	printf("A musica tem mais %d milissegundos restantes (%f segundos)\n",timeleft_ms,timeleft_s);

}
static void print_music_info(void){
	const char* file_name=Mix_GetMusicTitle(music);
	const char* artist_name=Mix_GetMusicArtistTag(music);
	const char* music_name=Mix_GetMusicTitleTag(music);
	const char* album_name=Mix_GetMusicAlbumTag(music);
	const char* copyright_info=Mix_GetMusicCopyrightTag(music);
	int duration_ms=((int)(Mix_MusicDuration(music)*1000.0));
	printf(MUSIC_INFO_FORMAT,file_name,duration_ms/1000,artist_name,album_name,music_name,copyright_info);

}
static void reset(void){
	
	Mix_RewindMusic();

}
static void join_controllers(void){

	pthread_join(menu_controller.thread_id,NULL);


}
static void cleanup(int useless){
		 join_controllers();
                 delete_music();
                 cleanSDL();
                 exit(useless);

}

static void stop(void){

	
	cleanup(-1);
}

static void menu_guts(char c){

	switch(c){

		case 'g':
		pthread_mutex_lock(&menu_controller.mtx);
		toggle();
		pthread_mutex_unlock(&menu_controller.mtx);
		break;
		case 's':
		pthread_mutex_lock(&menu_controller.mtx);
		stop();
		pthread_mutex_unlock(&menu_controller.mtx);
		break;
		case 'r':
		pthread_mutex_lock(&menu_controller.mtx);
		reset();
		pthread_mutex_unlock(&menu_controller.mtx);
		break;
		case 't':
		pthread_mutex_lock(&menu_controller.mtx);
		check_time_left();
		pthread_mutex_unlock(&menu_controller.mtx);
		break;
		case 'i':
		pthread_mutex_lock(&menu_controller.mtx);
		print_music_info();
		pthread_mutex_unlock(&menu_controller.mtx);
		break;
		default:
		break;

	}


}



static void* menu_loop(void* mem){
	char input=0;
	print_menu();
		
	do{
		if(!menu_running){

			break;
		}
		scanf("%c",&input);
		if(receive_cmd(input)){

			menu_guts(input);

		}
		else{

			printf("Input invalida\n");
		}



	}while(input!='s');

	return NULL;



}

static void init_controllers(void){

	pthread_create(&menu_controller.thread_id,NULL,menu_loop,NULL);
	menu_controller.stop_go=0;


}

int player_init(const char* filename){
	
	signal(SIGINT,cleanup);
	initSDL();

	prep_music(filename);
	printf("Funciona sim senhor\n");
	init_controllers();
	duration_ms= ((int)(Mix_MusicDuration(music)*1000.0));
	SDL_Delay(duration_ms);

	raise(SIGINT);

	return 0;
}

