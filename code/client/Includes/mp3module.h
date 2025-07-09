#ifndef MP_MODULE_H
#define MP_MODULE_H

typedef enum dec_op{D_DECODE_CHUNK_SIMPLE,D_DECODE_CHUNK_WITH_PRINT,D_SWAP,D_CLEAN,D_IS_D_BUFFER_EMPTY,D_IS_P_BUFFER_FULL,D_RESET_MP3_PTR,D_RESET_PCM_PTR,D_RESET_BOTH}dec_op;

typedef struct mp3decoder{

	pthread_mutex_t* mtx;
	mp3dec_t dec;
	uint8_t* r_chunk;
	uint8_t* d_chunk;
	uint8_t* p_chunk;
	uint32_t d_chunk_size;
	uint32_t p_chunk_size;
	uint32_t d_buffer_pos_cursor;
	uint32_t p_buffer_pos_cursor;

}mp3decoder;

int init_mp3_decoder(mp3decoder* decoder,uint32_t d_chunk_size,uint32_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff);
int perform_dec_op(mp3decoder* decoder,mp3decoder_result_struct* result,dec_op op);
#endif
