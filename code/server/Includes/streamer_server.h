#ifndef STREAMER_SERVER_H
#define STREAMER_SERVER_H
typedef struct stream_server_mem{

	int stream_on;
	int sockfd,sound_data;
	unsigned char raw_data[CHUNK_SIZE];
	char client_reply_buff[DEF_DATASIZE+1];
	int_pair pair;

	
}stream_server_mem;
extern stream_server_mem stream_struct;
//strings are null terminated!

void begin_stream(int client_sock,int fd,int_pair tpair);

void close_stream(void);
#endif
