#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#define MPG123_ENUM_API
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/mp3module.h"

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

static int is_decoder_buffer_empty(decoder_t* decoder){


	return decoder->d_buffer_pos_cursor<=0;


}
static int is_decoder_buffer_full(decoder_t* decoder){


	return decoder->d_buffer_pos_cursor>=decoder->d_chunk_size;


}
static int is_play_buffer_full(decoder_t* decoder){


	return decoder->p_buffer_pos_cursor>=(decoder->p_chunk_size);


}

static int is_play_buffer_empty(decoder_t* decoder){


	return decoder->p_buffer_pos_cursor<=0;


}

static void reset_decoder_state(decoder_t* decoder,dec_op op){
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
static int decode_chunk(decoder_t*dec,decoding_option what_we_want){
	int ret_val=MPG123_NEED_MORE;
	decoder_result_struct * result = &(((mp3_processed_chunk*)dec->p_chunk)->result_struct);
	switch(what_we_want){
		case DO_DECODE:
			ret_val= mpg123_decode(dec->dec,dec->d_chunk+4+sizeof(frame_info_t),((mp3_stream_chunk*)dec->d_chunk)->the_frame_info.size,dec->p_chunk+sizeof(decoder_result_struct)+4+dec->p_buffer_pos_cursor,dec->p_chunk_size-4-sizeof(decoder_result_struct),&result->total_bytes_in_chunk);
		   	if (ret_val == MPG123_OK || ret_val == MPG123_NEW_FORMAT) {
		        	mpg123_getformat(dec->dec, &result->hz, &result->channels, &result->encoding);
				result->sample_size=mpg123_encsize(result->encoding);
			}
			if(result->total_bytes_in_chunk){
				result->nsamples=(result->total_bytes_in_chunk)/(result->sample_size*result->channels);
		   	}
			result->decoder_state=(result->total_bytes_in_chunk>0);
			break;
		default:
			break;
	}
	return ret_val;

}
static void decoder_clean(decoder_t* decoder){

	if(decoder->dec){
		mpg123_close(decoder->dec);
		mpg123_delete(decoder->dec);
		mpg123_exit();
	}


}
static void swap_read_decode_buffs(decoder_t* decoder){
	
	uint8_t* tmp=decoder->p_chunk;
	decoder->p_chunk=decoder->d_chunk;
	decoder->d_chunk=tmp;




}
int perform_dec_op(decoder_t* decoder,dec_op op,decoding_option option){
	int res=-1;
	pthread_mutex_lock(decoder->mtx);
	switch(op){
		case D_DECODE_CHUNK:
			res=decode_chunk(decoder,option);
			break;
		case D_IS_D_BUFFER_EMPTY:
			res=is_decoder_buffer_empty(decoder);
			break;
		case D_IS_P_BUFFER_FULL:
			res=is_play_buffer_full(decoder);
			break;
		case D_IS_D_BUFFER_FULL:
			res=is_decoder_buffer_full(decoder);
			break;
		case D_IS_P_BUFFER_EMPTY:
			res=is_play_buffer_empty(decoder);
			break;
		case D_D_BUFFER_CURSOR:
			res=decoder->d_buffer_pos_cursor;
			break;
		case D_P_BUFFER_CURSOR:
			res=decoder->p_buffer_pos_cursor;
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
			decoder_clean(decoder);
			res=0;
			break;
		default:
			break;
	}
	pthread_mutex_unlock(decoder->mtx);
	return res;
}
int init_decoder(decoder_t* decoder,uint64_t d_chunk_size,uint64_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff){

	decoder->mtx=&mtx;
	decoder->r_chunk=r_buff;
	decoder->d_chunk=d_buff;
	decoder->p_chunk=p_buff;
	decoder->d_chunk_size=d_chunk_size;
	decoder->p_chunk_size=p_chunk_size;
	decoder->d_buffer_pos_cursor=0;
	decoder->p_buffer_pos_cursor=0;
	mpg123_init();
	decoder->dec=mpg123_new(NULL,NULL);
	mpg123_param(decoder->dec, MPG123_ADD_FLAGS, MPG123_QUIET,0.0);
	mpg123_param(decoder->dec, MPG123_RESYNC_LIMIT, 4096,0.0);
	mpg123_open_feed(decoder->dec);
	return 0;

}

