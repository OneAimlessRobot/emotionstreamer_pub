#include "../Includes/preprocessor.h"
/*#include "../../player/SDL2/include/SDL.h"
#include "../../player/SDL2/include/SDL_mixer.h"
#include "../../player/SDL2/include/SDL_rwops.h"
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../Includes/menu.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/streamer_client.h"
/*



Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
Mix_PlayChannel(-1, stream_struct.chk, 0);



*/

//ESTE CODIGO N E MEU!!!!!

//--------------------------------------------------------
//https://discourse.libsdl.org/t/time-length-of-sdl-mixer-chunks/12852

/* untested code follows… */
uint32_t getChunkTimeMilliseconds(Mix_Chunk*chunk)
{
double points = 0.0;
double frames = 0.0;
int freq = 0;
uint16_t fmt = 0;
int chans = 0;
/*Chunks are converted to audio device format… */
if (!Mix_QuerySpec(&freq, &fmt, &chans))
	return 0;
/*never called Mix_OpenAudio()?! */

 /* bytes / samplesize == sample points */
 points = ((double)chunk->alen / (double)((fmt & 0xFF) / 8));

 /* sample points / channels == sample frames */
 frames = (points / (double)chans);

 /* (sample frames * 1000) / frequency == play length in ms */
 return round(((frames * 1000) / (double)freq));
}
//--------------------------------------------------------


static stream_client_mem stream_struct={
                                     	0,
                                        0,
					0,
                                        NULL,
                                        {0},
					{0},
					{0}};




static void zero_chk_cache(void){


	memset(stream_struct.chunk_data_cache,0, CHUNK_SIZE*STREAM_CACHE_SIZE_CHUNKS);

}
static void play_chk_cache(void){

	int delay_time_cache=getChunkTimeMilliseconds(stream_struct.chk);
	Mix_PlayChannel(-1, stream_struct.chk, 0);
	SDL_Delay(delay_time_cache);
	Mix_FreeChunk(stream_struct.chk);
	stream_struct.chk=NULL;
}
static void cleanSDL(void){

	if(stream_struct.chk){

		Mix_FreeChunk(stream_struct.chk);
	}
	
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

}
static void initSDL(void){
	
	if(SDL_Init(SDL_INIT_AUDIO)){
		fprintf(logstream,"Erro a inicializar SDL!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	if(!Mix_Init(ALL_MIXER_FLAGS)){
		fprintf(logstream,"Erro a inicializar SDL_MIXER!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	if(Mix_OpenAudio(FREQ,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,CHUNK_SIZE)){
		fprintf(logstream,"Erro a abrir SDL AUDIO!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %d - %s\n",errno, strerror(errno));
		exit(-1);
	}
	Mix_Volume(-1,128);



}
static void play_lingering(void){

	if((stream_struct.curr_chk_index)&&((stream_struct.curr_chk_index)%STREAM_CACHE_SIZE_CHUNKS)){
		int remaining_len_bytes=((stream_struct.curr_chk_index)%STREAM_CACHE_SIZE_CHUNKS)*CHUNK_SIZE;
		
		stream_struct.chk=Mix_QuickLoad_RAW(stream_struct.chunk_data_cache,remaining_len_bytes);
		int delay_time_cache=getChunkTimeMilliseconds(stream_struct.chk);
		Mix_PlayChannel(-1, stream_struct.chk, 0);
		SDL_Delay(delay_time_cache);
		Mix_FreeChunk(stream_struct.chk);
		stream_struct.chk=NULL;
	}

}
static void stop(void){

	cleanSDL();


}
static void cleanup(int useless){

	stream_struct.stream_on=0;

}
static void init_stream(int client_sock,int_pair pair){

         stream_struct.stream_on=1;
         stream_struct.sockfd=client_sock;
         memcpy((void*)stream_struct.pair,pair,sizeof(int_pair));

}
static int process_chunk_cache(void){
	memset(stream_struct.server_reply_buff,0,DEF_DATASIZE+1);

	if(!(stream_struct.curr_chk_index)||((stream_struct.curr_chk_index)%STREAM_CACHE_SIZE_CHUNKS)){

		snprintf(stream_struct.server_reply_buff,DEF_DATASIZE,"%s",DEFAULT_MORE_STRING);


		stream_struct.curr_chk_index++;
		sendsome(stream_struct.sockfd,(char*)stream_struct.server_reply_buff,DEF_DATASIZE,stream_struct.pair);
	
	}
	else{

		stream_struct.chk=Mix_QuickLoad_RAW(stream_struct.chunk_data_cache,CHUNK_SIZE*STREAM_CACHE_SIZE_CHUNKS);
		int delay_time_cache=getChunkTimeMilliseconds(stream_struct.chk);
		snprintf(stream_struct.server_reply_buff,DEF_DATASIZE,"%s",DEFAULT_ENOUGH_STRING);
		sendsome(stream_struct.sockfd,(char*)stream_struct.server_reply_buff,DEF_DATASIZE,stream_struct.pair);
		memset(stream_struct.server_reply_buff,0,DEF_DATASIZE+1);
		
		readsome(stream_struct.sockfd,(char*)stream_struct.server_reply_buff,DEF_DATASIZE,stream_struct.pair);
		

		if(strs_are_strictly_equal(DEFAULT_HOWLONG_STRING,stream_struct.server_reply_buff)){

			printf("Resposta de server estranha!!!!!\nResposta de server recebida: %s\nResposta esperada: %s\n",stream_struct.server_reply_buff,DEFAULT_HOWLONG_STRING);
			return 0;
		}

		memset(stream_struct.server_reply_buff,0,DEF_DATASIZE+1);
		snprintf(stream_struct.server_reply_buff,DEF_DATASIZE,"%d",delay_time_cache);
		sendsome(stream_struct.sockfd,(char*)stream_struct.server_reply_buff,DEF_DATASIZE,stream_struct.pair);
		play_chk_cache();
		zero_chk_cache();
		stream_struct.curr_chk_index=0;
	}
	
	return 1;


	

}
static int read_play_chunk_func(void){


        int result= readsome(stream_struct.sockfd,(char*)stream_struct.chunk_data_cache+((stream_struct.curr_chk_index)%STREAM_CACHE_SIZE_CHUNKS)*CHUNK_SIZE,CHUNK_SIZE,stream_struct.pair);
	process_chunk_cache();

	return result;

}



static void stream_func(void){

	int result=0;
	while(stream_struct.stream_on&&((result=read_play_chunk_func())>0)){}
	
	play_lingering();

}

int player_init_stream(int server_sock){


	signal(SIGPIPE, cleanup);

	signal(SIGINT, cleanup);

	initSDL();
	init_stream(server_sock,CLIENT_DATA_TIMES_PAIR);
	stream_func();

	stop();

	return 0;
}


