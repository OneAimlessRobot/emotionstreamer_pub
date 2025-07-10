#ifndef RIPPED_CODE_H
#define RIPPED_CODE_H

typedef struct
{
     int decoder_state;
     uint8_t stream_marker;
     uint8_t metadata_block_type;
     uint32_t frame_bytes;
     uint64_t chunk_id;
     uint32_t metadata_length;
     uint64_t decoder_in_chunk_size;
     uint64_t decoder_out_chunk_size;
     uint8_t channels;
     uint32_t hz;
     uint8_t bitrate_kbps;
     uint8_t is_last_metadata;
     uint32_t total_samples;
     uint32_t nsamples;
     uint64_t dec_input_chunk_ptr;
     uint64_t dec_output_chunk_ptr;
} decoder_result_struct;

typedef struct chunk_size_helper{
	int32_t freq,chans;
	uint16_t fmt;
	uint32_t audio_len;


}chunk_size_helper;



int should_switch(decoder_result_struct* before_result,decoder_result_struct* current_result);
uint32_t getChunkTimeMilliseconds(chunk_size_helper* helper);
int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* buff_to_play,decoder_result_struct* result);
int play_from_sound_device_pa(pa_simple* handle,uint8_t* buff_to_play,decoder_result_struct*result);
void print_decoder_frame_result(decoder_result_struct* result,int fd);
#endif
