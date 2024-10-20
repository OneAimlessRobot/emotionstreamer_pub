#include "../Includes/preprocessor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../Includes/menu.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
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
                                        NULL,
                                        };

static void zero_chk_cache(client_stream_t* strm){


        memset(strm->chunk_data_cache,0, cfg_chunk_size);


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
	if(strm->innited){
       		cleanSDL(strm);
        	printf("SAIMOS DA STREAM DO CLIENTE! Vamos ver errno:%s\n",strerror(errno));
		close_con(strm->con_obj);
		strm->innited=0;
	}
	
}


static void sigint_handler(int useless){


	stop_client_stream(&stream_struct + (0*useless));
	exit(useless);

}

static void sigpipe_handler(int useless){

	sigint_handler(useless);

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
	strm->curr_chk_index+=cfg_chunk_size;
	if((strm->curr_chk_index)==(cfg_chunk_size*cfg_stream_cache_size_chunks)){
		make_chunk(strm);
        	wait=1;
		strm->curr_chk_index=0;

	}
        return wait;
}

static int read_chunk_tcp(client_stream_t* strm,int_pair pair){

	 return readsome(strm->con_obj->sockfd_tcp,(char*)(strm->chunk_data_cache+strm->curr_chk_index),cfg_chunk_size,pair);

}
static int read_chunk_udp(client_stream_t* strm,int_pair pair){

        return readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->chunk_data_cache+strm->curr_chk_index),cfg_chunk_size,pair,&strm->con_obj->peer_udp_addr);
}


int get_server_chunk(client_stream_t* strm,int_pair pair){

        int result= read_chunk_udp(strm, pair);
	if(result<0){

                return result;
	}
	int wait=process_chunk(strm);
	snprintf((char*)strm->con_obj->ack_udp_data,cfg_datasize,"%s %d",ACK_STRING,wait);
	result=con_send_udp_ack(strm->con_obj,pair);
        if(result<0){

                return result;
	}
	if(wait){
		play_chunk(strm);
		result= con_send_udp_ack(strm->con_obj,server_drop_chunks_times_pair);
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
        if(Mix_OpenAudio(cfg_freq,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,cfg_chunk_size)){
                fprintf(logstream,"Erro a abrir SDL AUDIO!!!!!!! Isto vai parar!!!!\n Não foi possivel tocar o ficheiro!!!!\n Erro: %s\n",strerror(errno));
                exit(-1);
        }
        strm->innited=1;
        Mix_Volume(-1,128);



}

static void client_stream(client_stream_t* strm){

        while(strm->con_obj->is_on&&(get_server_chunk(strm,client_data_times_pair)>0)){}
	

}

static int init_client_stream(client_stream_t* strm,con_t* con_obj, unsigned char* buff){
	
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	initSDL(strm);
	strm->chunk_data_cache=buff;
	memset(strm->chunk_data_cache,0,cfg_chunk_size*cfg_stream_cache_size_chunks);
	strm->con_obj=con_obj;
        client_stream(strm);
        raise(SIGINT);
	return 0;
}



void player_stop_stream(void){
	raise(SIGINT);
}

void player_init_stream(con_t* con_obj,unsigned char* buff){

	init_client_stream(&stream_struct,con_obj,buff);
}

