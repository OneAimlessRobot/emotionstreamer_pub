#ifndef RIPPED_CODE_H
#define RIPPED_CODE_H

typedef struct
{
     uint64_t chunk_id;
     uint32_t decoder_in_chunk_size;
     uint32_t decoder_out_chunk_size;
     int frame_bytes;
     int channels;
     int hz;
     int layer;
     int bitrate_kbps;
     uint32_t nsamples;
     uint32_t dec_input_chunk_ptr;
     uint32_t dec_output_chunk_ptr;
} mp3decoder_result_struct;

typedef struct chunk_size_helper{
	int32_t freq,chans;
	uint16_t fmt;
	uint32_t audio_len;


}chunk_size_helper;



int should_switch(mp3decoder_result_struct* before_result,mp3decoder_result_struct* current_result);
uint32_t getChunkTimeMilliseconds(chunk_size_helper* helper);
int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* buff_to_play,mp3decoder_result_struct* result);
int play_from_sound_device_pa(pa_simple* handle,uint8_t* buff_to_play,mp3decoder_result_struct*result);
void print_decoder_frame_result(mp3decoder_result_struct* result,int fd);
#endif
