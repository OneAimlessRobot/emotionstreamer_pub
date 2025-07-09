#include "../../Includes/preprocessor.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include <sys/ioctl.h> //for ioctl()
#include <linux/soundcard.h> //SOUND_PCM*
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include <alsa/asoundlib.h>
#include "../Includes/ripped_code.h"
#include "../../minimp3/minimp3.h"
#include "../Includes/mp3module.h"


/*
static void print_chunk_helper_info(chunk_size_helper* helper){
	printf("Info deste helper: Frequencia: %d\nChannels: %d\nFormato: %u\nAudio len %u\n",helper->freq,helper->chans,helper->fmt,helper->audio_len);

}
*/

//https://discourse.libsdl.org/t/time-length-of-sdl-mixer-chunks/12852/2
/* untested code follows… */

void print_decoder_frame_result(mp3decoder_result_struct* result,int fd){

	if(result){
		dprintf(fd,"Estes sao os dados deste frame:\n"
							"\nresult->chunk_id: %ld\n"
							"\nresult->frame_bytes: %d\n"
							"result->channels: %d"
							"\nresult->hz: %d"
							"\nresult->layer: %d"
							"\nresult->bitrate_kbps: %d"
							"\nresult->nsamples: %hd\n"
							"\nresult->dec_input_chunk_ptr: %d\n"
							"\nresult->dec_output_chunk_ptr: %d\n",
							result->chunk_id,
							result->frame_bytes,
							result->channels,
							result->hz,
							result->layer,
							result->bitrate_kbps,
							result->nsamples,
							result->dec_input_chunk_ptr,
							result->dec_output_chunk_ptr);
	}

}

uint32_t getChunkTimeMilliseconds(chunk_size_helper* helper)
{
double points = 0.0;
double frames = 0.0;

 /* bytes / samplesize == sample points */
 points = ((double) helper->audio_len / (double) ((helper->fmt & 0xFF) / 8));

 /* sample points / channels == sample frames */
 frames = ((double) points / (double) helper->chans);

 /* (sample frames * 1000) / frequency == play length in ms */
 int time=round((frames * 1000) / (double) helper->freq);

 //print_chunk_helper_info(helper);
 //printf("Time of this chunk: %dms\nNumero de samples: %lf\nNumero de frames: %lf\n",time,points,frames);
 return time;

}


/*This program demonstrates how to read in a raw
  data file and write it to the sound device to be played.
  The program uses the ALSA library.
  Use option -lasound on compile line.*/
 
int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* sound_buff_to_play,mp3decoder_result_struct* result)
{
  int err;
  snd_pcm_sframes_t frames;
  frames = snd_pcm_writei(handle,sound_buff_to_play, result->nsamples*SIZE*result->channels);
  if (frames < 0){
	frames = snd_pcm_recover(handle, frames, 0);
  }
  if ((err=frames) < 0) {
    	fprintf(stderr, "snd_pcm_writei failed to play %ld bytes:\nsize=%ld\nnchannels=%d\nnsamples=%d\n",result->nsamples*result->channels*SIZE,SIZE,result->channels,result->nsamples);
	
  }
  return 0;
}

int play_from_sound_device_pa(pa_simple* handle,uint8_t* sound_buff_to_play,mp3decoder_result_struct* result)
{
    //if (pa_simple_write(handle, sound_buff_to_play, result->nsamples*result->channels*SIZE, NULL) < 0) {
    if (pa_simple_write(handle, sound_buff_to_play, 1152*2*SIZE, NULL) < 0) {
        fprintf(stderr, "pa_simple_write() failed to play %ld bytes: \nsize=%ld\nnchannels=%d\nnsamples=%d\n%s",result->nsamples*result->channels*SIZE,SIZE,result->channels,result->nsamples,pa_strerror(errno));
	//print_decoder_frame_result(result,1);
	return 1;
    }

    return 0;
}
