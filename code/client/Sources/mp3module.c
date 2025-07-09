#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"

#define MINIMP3_IMPLEMENTATION

#include "../../minimp3/minimp3.h"
#include "../Includes/mp3module.h"
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static uint64_t curr_chunk_id=0;
/*
static void clean_decoder(chunk_player* player){


}
*/

static int is_decoder_buffer_empty(mp3decoder* decoder){


	return decoder->d_buffer_pos_cursor>=decoder->d_chunk_size;


}
static int is_play_buffer_full(mp3decoder* decoder){


	return decoder->p_buffer_pos_cursor>=(decoder->p_chunk_size/2);


}
static void decode_chunk(mp3decoder*decoder,mp3decoder_result_struct* result,int with_print){

	mp3dec_frame_info_t info={0};
	int16_t nsamples=mp3dec_decode_frame(&decoder->dec, decoder->d_chunk+decoder->d_buffer_pos_cursor, decoder->d_chunk_size-decoder->d_buffer_pos_cursor, (int16_t*)(decoder->p_chunk+(decoder->p_buffer_pos_cursor)), &info);
	result->chunk_id=curr_chunk_id;
	result->frame_bytes=info.frame_bytes;
	result->channels=info.channels;
	result->hz=info.hz;
	result->layer=info.layer;
	result->bitrate_kbps=info.bitrate_kbps;
	result->nsamples=nsamples;
	if(with_print){
		print_decoder_frame_result(result,1);
	}
	decoder->d_buffer_pos_cursor=min((decoder->d_buffer_pos_cursor+result->frame_bytes),decoder->d_chunk_size);
	decoder->p_buffer_pos_cursor=min((decoder->p_buffer_pos_cursor+result->nsamples),decoder->p_chunk_size/2);
	result->dec_input_chunk_ptr=decoder->d_buffer_pos_cursor;
	result->dec_output_chunk_ptr=decoder->p_buffer_pos_cursor;
	curr_chunk_id++;
}
static void reset_decoder_state(mp3decoder* decoder,dec_op op){
	if(decoder){
		switch(op){
			case D_RESET_BOTH:
			if(is_decoder_buffer_empty(decoder)){
				decoder->d_buffer_pos_cursor=0;

			}
			if(is_play_buffer_full(decoder)){
				decoder->p_buffer_pos_cursor=0;
			}
			break;
			case D_RESET_MP3_PTR:
			if(is_decoder_buffer_empty(decoder)){
				decoder->d_buffer_pos_cursor=0;

			}
			break;
			case D_RESET_PCM_PTR:
			if(is_play_buffer_full(decoder)){
				decoder->p_buffer_pos_cursor=0;
			}
			break;
			default:
			break;
		}
	}
}
static void swap_read_decode_buffs(mp3decoder* decoder){
	
	uint8_t* tmp=decoder->p_chunk;
	decoder->p_chunk=decoder->d_chunk;
	decoder->d_chunk=tmp;




}
int perform_dec_op(mp3decoder* decoder,mp3decoder_result_struct* result,dec_op op){
	
	int res=-1;
	pthread_mutex_lock(decoder->mtx);
	switch(op){
		case D_DECODE_CHUNK_SIMPLE:
			decode_chunk(decoder,result,0);
			res=0;
			break;
		case D_DECODE_CHUNK_WITH_PRINT:
			decode_chunk(decoder,result,1);
			res=0;
			break;
		case D_IS_D_BUFFER_EMPTY:
			res=is_decoder_buffer_empty(decoder);
			break;
		case D_IS_P_BUFFER_FULL:
			res=is_play_buffer_full(decoder);
			break;
		case D_SWAP:
			swap_read_decode_buffs(decoder);
			res=0;
			break;
		case D_RESET_MP3_PTR:
			reset_decoder_state(decoder,op);
			res=0;
			break;
		case D_RESET_PCM_PTR:
			reset_decoder_state(decoder,op);
			res=0;
			break;
		case D_RESET_BOTH:
			reset_decoder_state(decoder,op);
			res=0;
			break;
		case D_CLEAN:
			res=0;
			break;
		default:
			break;
	}
	pthread_mutex_unlock(decoder->mtx);
	return res;
}
int init_mp3_decoder(mp3decoder* decoder,uint32_t d_chunk_size,uint32_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff){

	decoder->mtx=&mtx;
	decoder->r_chunk=r_buff;
	decoder->d_chunk=d_buff;
	decoder->p_chunk=p_buff;
	decoder->d_chunk_size=d_chunk_size;
	decoder->p_chunk_size=p_chunk_size;
	decoder->d_buffer_pos_cursor=0;
	decoder->p_buffer_pos_cursor=0;
	mp3dec_init(&decoder->dec);
	return 0;

}

