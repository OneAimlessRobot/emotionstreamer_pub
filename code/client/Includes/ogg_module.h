#ifndef OGG_MODULE_H
#define OGG_MODULE_H
typedef enum dec_op{D_DECODE_CHUNK,D_SWAP,D_CLEAN,D_IS_D_BUFFER_EMPTY,D_IS_P_BUFFER_FULL,D_RESET_MP3_PTR,D_RESET_PCM_PTR,D_RESET_BOTH}dec_op;
typedef enum decoding_option{DO_FEED,DO_DECODE,DO_READ}decoding_option;

#define DECODER_BUFFER_SIZE_IN_CHUNKS 100

#define MAXIMUM_SIZE_OGG_OUTPUT_BUFFER (SIZE*SAMPLES*CHANNELS)
typedef struct decoder{

	pthread_mutex_t* mtx;
	mpg123_handle *dec;
	uint8_t* r_chunk;
	uint8_t* h2_chunk;
	uint8_t* d_chunk;
	uint8_t* p_chunk;
	uint64_t d_chunk_size;
	uint64_t p_chunk_size;
	uint64_t d_buffer_pos_cursor;
	uint64_t p_buffer_pos_cursor;
	
}decoder;

int init_decoder(decoder* decoder,uint64_t d_chunk_size,uint64_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff,uint8_t* h2_buff);
int perform_dec_op(decoder* decoder,frame_info_t* finfo,decoder_result_struct* result,dec_op op,decoding_option option);

#endif
