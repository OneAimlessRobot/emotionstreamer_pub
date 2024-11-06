#ifndef RIPPED_CODE_H
#define RIPPED_CODE_H

typedef struct chunk_size_helper{
	int32_t freq,chans;
	uint16_t fmt;
	uint32_t audio_len;


}chunk_size_helper;



uint32_t getChunkTimeMilliseconds(chunk_size_helper* helper);
int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* sound_buff_to_play,int size);
int play_from_sound_device_pa(pa_simple* handle,uint8_t* sound_buff_to_play,int size);
#endif
