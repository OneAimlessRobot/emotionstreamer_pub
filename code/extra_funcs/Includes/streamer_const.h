#ifndef STREAMER_CONST_H
#define STREAMER_CONST_H

#define SERVER_DEF_CHUNK_SIZE 8192

#define STREAM_DEF_DECODE_CACHE_SIZE_CHUNKS 37
#define STREAM_DEF_PLAYER_CACHE_SIZE_CHUNKS 37
#define DEF_LATENCY_MS 4000

#define MAX_MP3_STREAM_CHUNK_BUFF_SIZE 32768

typedef struct mp3_stream_chunk{
	
	frame_info_t the_frame_info;
	uint8_t padding[4];
	uint8_t actual_frame[MAX_MP3_STREAM_CHUNK_BUFF_SIZE+1];

}mp3_stream_chunk;

#endif
