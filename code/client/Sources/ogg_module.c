#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"

#define MINIFLAC_IMPLEMENTATION
#define MINIFLAC_API
#define MINIFLAC_PRIVATE static inline

#include "../../miniflac/miniflac.h"
#include "../Includes/ogg_module.h"
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

static uint64_t curr_chunk_id=0;

static int is_decoder_buffer_empty(decoder* decoder){


	return decoder->d_buffer_pos_cursor>=decoder->d_chunk_size;


}
static int is_play_buffer_full(decoder* decoder){


	return decoder->p_buffer_pos_cursor>=(decoder->p_chunk_size);


}
static int decode_chunk(decoder*decoder,decoder_result_struct* result,decoding_option what_we_want){


	int32_t multi_channel[CHANNELS][SAMPLES];
	int32_t* arr_of_ptrs[CHANNELS]={0};
	uint32_t current_sub_advance=0;
	MINIFLAC_RESULT return_val=0;
	switch(what_we_want){
		case DO_DECODE:
			for(uint32_t i=0;i<CHANNELS;i++){
				
				arr_of_ptrs[i]=(int32_t*)(&(multi_channel[i]));
			}
			return_val=miniflac_decode(&decoder->dec, decoder->d_chunk+decoder->d_buffer_pos_cursor,decoder->d_chunk_size-decoder->d_buffer_pos_cursor, &(current_sub_advance),(int32_t**)&arr_of_ptrs);
			result->decoder_state=decoder->dec.state;
			result->hz=decoder->dec.frame.header.sample_rate;
			result->bitrate_kbps=decoder->dec.frame.header.bps;
			result->chunk_id=curr_chunk_id;
			result->channels=decoder->dec.frame.header.channels;
			result->frame_bytes=current_sub_advance;
			result->decoder_in_chunk_size=decoder->d_chunk_size;
			result->decoder_out_chunk_size=decoder->p_chunk_size;
			if(result->frame_bytes){
				result->nsamples=decoder->dec.frame.size/(SIZE*result->channels);
				for (uint32_t i = 0; i < result->nsamples; i++) {
					for(uint32_t j=0;(j<result->channels)&&(j<CHANNELS);j++){
						decoder->p_chunk[(j+i*result->channels)] = multi_channel[j][i];
					}
				}
			}
			break;
		case DO_SYNC:
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk+result->frame_bytes,4, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;
			result->metadata_block_type=decoder->dec.metadata.header.type;
			result->metadata_length=decoder->dec.metadata.header.length;
			result->metadata_length=decoder->dec.metadata.header.is_last_metadata;
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk+result->frame_bytes,4, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;
			result->metadata_length=decoder->dec.metadata.header.length;
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk+result->frame_bytes,26, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;

			result->decoder_state=decoder->dec.state;
			break;
		case DO_STREAMINFO:
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk,4, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;
			result->stream_marker=decoder->dec.streammarker.state;
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk+result->frame_bytes,4, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;
			result->metadata_length=decoder->dec.metadata.header.length;
			result->metadata_length=decoder->dec.metadata.header.is_last_metadata;
			return_val=miniflac_sync(&decoder->dec, decoder->d_chunk+result->frame_bytes,26, &(current_sub_advance));
			result->frame_bytes+=current_sub_advance;

			result->decoder_state=decoder->dec.state;
			result->hz=decoder->dec.metadata.streaminfo.sample_rate;
			result->bitrate_kbps=decoder->dec.metadata.streaminfo.bps;
			result->chunk_id=curr_chunk_id;
			result->channels=decoder->dec.metadata.streaminfo.bps;
			result->decoder_in_chunk_size=decoder->d_chunk_size;
			result->decoder_out_chunk_size=decoder->p_chunk_size;
			break;

	}
	decoder->d_buffer_pos_cursor=min((decoder->d_buffer_pos_cursor+result->frame_bytes),decoder->d_chunk_size);
	decoder->p_buffer_pos_cursor=min((decoder->p_buffer_pos_cursor+(result->nsamples*SIZE*result->channels)),decoder->p_chunk_size);
	result->dec_input_chunk_ptr=decoder->d_buffer_pos_cursor;
	result->dec_output_chunk_ptr=decoder->p_buffer_pos_cursor;
	curr_chunk_id++;
	return return_val;
}
static void reset_decoder_state(decoder* decoder,dec_op op){
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
static void swap_read_decode_buffs(decoder* decoder){
	
	uint8_t* tmp=decoder->p_chunk;
	decoder->p_chunk=decoder->d_chunk;
	decoder->d_chunk=tmp;




}
int perform_dec_op(decoder* decoder,decoder_result_struct* result,dec_op op,decoding_option option){
	
	int res=-1;
	pthread_mutex_lock(decoder->mtx);
	switch(op){
		case D_DECODE_CHUNK:
			res=decode_chunk(decoder,result,option);
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
int init_decoder(decoder* decoder,uint64_t d_chunk_size,uint64_t p_chunk_size,uint8_t* r_buff,uint8_t* d_buff,uint8_t* p_buff){

	decoder->mtx=&mtx;
	decoder->r_chunk=r_buff;
	decoder->d_chunk=d_buff;
	decoder->p_chunk=p_buff;
	decoder->d_chunk_size=d_chunk_size;
	decoder->p_chunk_size=p_chunk_size;
	decoder->d_buffer_pos_cursor=0;
	decoder->p_buffer_pos_cursor=0;
	miniflac_init(&decoder->dec,0);
	return 0;

}

