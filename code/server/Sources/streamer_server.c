#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/streamer_server.h"

stream_server_mem stream_struct={
					0,
					0,
					0,
					{0},
					{0},
					{0}};


void close_stream(void){

	stream_struct.stream_on=0;
	close(stream_struct.sockfd);

}



static void init_stream(int client_sock, int fd, int_pair pair);




/*
extern DECLSPEC Mix_Chunk * SDLCALL Mix_QuickLoad_RAW(Uint8 *mem, Uint32 len);

		stream_struct.chk=Mix_QuickLoad_RAW(stream_struct.raw_data, sizeof(stream_struct.raw_data);


extern DECLSPEC void *SDLCALL SDL_LoadFile(const char *file, size_t *datasize);

extern DECLSPEC Sint64 SDLCALL SDL_RWseek(SDL_RWops *context,
                                          Sint64 offset, int whence);

extern DECLSPEC Sint64 SDLCALL SDL_RWtell(SDL_RWops *context);

extern DECLSPEC size_t SDLCALL SDL_RWread(SDL_RWops *context,
                                          void *ptr, size_t size,
                                          size_t maxnum);

	SDL_RWread(stream_struct.sound_data,(char*)(stream_struct.chk),sizeof(Mix_Chunk),1);

extern DECLSPEC SDL_RWops *SDLCALL SDL_RWFromFP(FILE * fp, SDL_bool autoclose);


extern DECLSPEC size_t SDLCALL SDL_RWwrite(SDL_RWops *context,
                                           const void *ptr, size_t size,
                                           size_t num);
*/

static void init_stream(int client_sock, int fd, int_pair pair){

	stream_struct.stream_on=1;
	stream_struct.sockfd=client_sock;
	memcpy((void*)stream_struct.pair,pair,sizeof(int_pair));
	stream_struct.sound_data=fd;


}

static void send_chunk_func(void){

	memset(stream_struct.client_reply_buff,0,DEF_DATASIZE+1);


	sendsome(stream_struct.sockfd,(char*)stream_struct.raw_data,CHUNK_SIZE ,stream_struct.pair);

	readsome(stream_struct.sockfd,(char*)stream_struct.client_reply_buff,DEF_DATASIZE,stream_struct.pair);

	if(!strs_are_strictly_equal(DEFAULT_MORE_STRING,stream_struct.client_reply_buff)){

	}
	else if(!strs_are_strictly_equal(DEFAULT_ENOUGH_STRING,stream_struct.client_reply_buff)){
		memset(stream_struct.client_reply_buff,0,DEF_DATASIZE+1);
		snprintf(stream_struct.client_reply_buff,DEF_DATASIZE,"%s",DEFAULT_HOWLONG_STRING);
		sendsome(stream_struct.sockfd,(char*) stream_struct.client_reply_buff,DEF_DATASIZE,stream_struct.pair);
		memset(stream_struct.client_reply_buff,0,DEF_DATASIZE+1);
		int time_to_wait_ms=0;
		readsome(stream_struct.sockfd,(char*) stream_struct.client_reply_buff,DEF_DATASIZE,stream_struct.pair);

		sscanf(stream_struct.client_reply_buff,"%d",&time_to_wait_ms);

		usleep(time_to_wait_ms*1000);

	}
	else{

		printf("Resposta estranha %s\n",stream_struct.client_reply_buff);
		raise(SIGINT);
	}
	memset(stream_struct.raw_data,0,CHUNK_SIZE);
	
}
static void stream(void){


	while((read(stream_struct.sound_data,stream_struct.raw_data,CHUNK_SIZE)>0)&&stream_struct.stream_on){

			send_chunk_func();

		}



}

void begin_stream(int client_sock,int fd,int_pair tpair){

	init_stream(client_sock,fd,tpair);
	
	stream();

}



