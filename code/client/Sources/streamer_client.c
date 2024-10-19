#include "../Includes/preprocessor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../Includes/menu.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/ripped_code.h"
#include "../Includes/streamer_client.h"

static client_stream_t stream_struct={
					0,
					NULL,
					0,
                                        NULL,
                                        {0}
                                        };

static void zero_chk_cache(client_stream_t* strm){


        memset(strm->chunk_data_cache,0, CHUNK_SIZE);


}

static void cleanSDL(client_stream_t* strm){

        if(strm->chk){

                Mix_FreeChunk(strm->chk);
        }

        Mix_CloseAudio();
        Mix_Quit();
        SDL_Quit();

}

static void stop_client_stream(client_stream_t* strm){

        close_con(strm->con_obj);
        cleanSDL(strm);
        printf("SAIMOS DA STREAM DO CLIENTE! Vamos ver errno:%s\n",strerror(errno));

}


static void make_chunk(client_stream_t* strm){

	strm->chk=Mix_QuickLoad_RAW(strm->chunk_data_cache,(strm->curr_chk_index));

}
static void play_chunk(client_stream_t* strm){

	int delay_time_cache=getChunkTimeMilliseconds(strm->chk);
        Mix_PlayChannel(-1, strm->chk, 0);
        SDL_Delay(delay_time_cache);
        Mix_FreeChunk(strm->chk);
        strm->chk=NULL;
        zero_chk_cache(strm);

}
static int process_chunk(client_stream_t* strm){


	int wait=0;
	strm->curr_chk_index+=CHUNK_SIZE;
	if((strm->curr_chk_index)==sizeof(strm->chunk_data_cache)){
		make_chunk(strm);
        	wait=1;
		strm->curr_chk_index=0;

	}
        snprintf((char*)strm->con_obj->data,DEF_DATASIZE,"%s %d",ACK_STRING,wait);
        return wait;
}

int get_server_chunk(client_stream_t* strm,int_pair pair){

        int result= readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->chunk_data_cache+strm->curr_chk_index),CHUNK_SIZE,pair,&strm->con_obj->peer_udp_addr);
        if(result<0){

                stop_client_stream(strm);

        }
	int wait=process_chunk(strm);
        result=con_send_tcp(strm->con_obj,pair);
        if(wait){
		play_chunk(strm);
		con_send_tcp(strm->con_obj,CLIENT_DATA_TIMES_PAIR);
	}
	if(result<0){

                stop_client_stream(strm);

        }
	return result;

}

static void initSDL(client_stream_t* strm){

        if(SDL_Init(SDL_INIT_AUDIO)){
                fprintf(logstream,"Erro a inicializar SDL!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %s\n",strerror(errno));
                exit(-1);
        }
        if(!Mix_Init(ALL_MIXER_FLAGS)){
                fprintf(logstream,"Erro a inicializar SDL_MIXER!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!! Erro: %s\n",strerror(errno));
                exit(-1);
        }
        if(Mix_OpenAudio(FREQ,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,CHUNK_SIZE)){
                fprintf(logstream,"Erro a abrir SDL AUDIO!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %s\n",strerror(errno));
                exit(-1);
        }
        strm->innited=1;
        Mix_Volume(-1,128);



}

static void client_stream(client_stream_t* strm){

        while(strm->con_obj->is_on&&(get_server_chunk(strm,CLIENT_DATA_TIMES_PAIR)>0)){}


}

static int init_client_stream(client_stream_t* strm,con_t* con_obj){


        strm->con_obj=con_obj;
        initSDL(strm);
        client_stream(strm);
        stop_client_stream(strm);
        return 0;
}



void player_stop_stream(void){
	stop_client_stream(&stream_struct);

}

int player_init_stream(con_t* con_obj){

	init_client_stream(&stream_struct,con_obj);

	return 0;
}

