#ifndef STREAMER_SERVER_H
#define STREAMER_SERVER_H


typedef struct server_stream_t{

	  int initted;
          uint16_t curr_timeout;
	  struct con_t* con_obj;
          int local_fd;
          //unsigned char chunk_data_cache[CHUNK_SIZE*STREAM_CACHE_SIZE_CHUNKS];
          unsigned char* chunk_data_cache;
	  uint16_t chunk_size;
}server_stream_t;


//strings are null terminated!
//con_obj is innited
//BUFF COMES FROM THE OUTSIDE!
//fds are valid
//DOES NOT FREE BUFFS OR INITIALIZE BUFFS
void begin_stream(con_t* con_obj,int fd,  uint16_t chunk_size,unsigned char* stream_buff);

void close_stream(void);
#endif
