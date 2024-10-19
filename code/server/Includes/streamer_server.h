#ifndef STREAMER_SERVER_H
#define STREAMER_SERVER_H


typedef struct server_stream_t{


          struct con_t* con_obj;
          int local_fd;
          //unsigned char chunk_data_cache[CHUNK_SIZE*STREAM_CACHE_SIZE_CHUNKS];
          unsigned char chunk_data_cache[CHUNK_SIZE];

}server_stream_t;


//strings are null terminated!
//objects are all innited
//fds are valid
void begin_stream(con_t* con_obj,int fd);

void close_stream(void);
#endif
