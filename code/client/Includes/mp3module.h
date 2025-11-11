#ifndef MP_MODULE_H
#define MP_MODULE_H
typedef enum dec_op{D_DECODE_CHUNK,D_SWAP,D_CLEAN,D_D_BUFFER_CURSOR,D_P_BUFFER_CURSOR,D_IS_D_BUFFER_EMPTY,D_IS_P_BUFFER_FULL,D_IS_P_BUFFER_EMPTY,D_IS_D_BUFFER_FULL,D_RESET_MP3_PTR,D_RESET_PCM_PTR,D_RESET_BOTH}dec_op;
typedef enum decoding_option{DO_FEED,DO_DECODE,DO_READ}decoding_option;

#define DECODER_BUFFER_SIZE_IN_CHUNKS 100

#define MAXIMUM_SIZE_OGG_OUTPUT_BUFFER (SIZE*SAMPLES*CHANNELS)
typedef struct decoder_t{

	pthread_mutex_t* mtx;
	mpg123_handle *dec;
	uint8_t* r_chunk;
	uint8_t* d_chunk;
	uint8_t* p_chunk;
	uint64_t d_chunk_size;
	uint64_t p_chunk_size;
	uint64_t d_buffer_pos_cursor;
	uint64_t p_buffer_pos_cursor;

}decoder_t;

typedef struct mp3_processed_chunk{
	decoder_result_struct result_struct;
	uint8_t padding[4];
	uint8_t decoded_frame_data[MAX_MP3_STREAM_CHUNK_BUFF_SIZE];
}mp3_processed_chunk;

int init_decoder(decoder_t* decoder,uint64_t d_chunk_size,uint64_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff);
int perform_dec_op(decoder_t* decoder,dec_op op,decoding_option option);

#endif
