#ifndef CHUNK_PLAYER_H
#define CHUNK_PLAYER_H
typedef enum method{PLAY_PA,PLAY_ALSA}method;

typedef enum play_op{P_REAL_PLAY,P_DRY_PLAY,P_PLAY_NA,P_CLEAN}play_op;


typedef struct chunk_player{
	pthread_mutex_t* mtx;
	snd_pcm_t*  play_stream_alsa;
	pa_simple* play_stream_pa;
	method which_mode;
	uint8_t* p_chunk;
	uint32_t chunk_size;

}chunk_player;


int init_chunk_player(chunk_player* player,uint16_t chunk_size,uint8_t* p_buff,method the_way);
void perform_play_op(chunk_player* player,mp3decoder_result_struct* result,play_op op);

#endif
