#include "../../Includes/preprocessor.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <ncurses.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <sys/ioctl.h> //for ioctl()
#include <linux/soundcard.h> //SOUND_PCM*
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include <ao/ao.h>
#include "../Includes/configs.h"
#include <alsa/asoundlib.h>
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/ripped_code.h"
#include "../Includes/mp3module.h"
static pthread_mutex_t ncurses_mtx=PTHREAD_MUTEX_INITIALIZER;

static const char* frame_print_format="Estes sao os dados deste frame:\n"
							"\nValor de MAXIMUM_SIZE_OGG_OUTPUT_BUFFER: %lu"
							"\nresult->metadata_block_type: %hu"
							"\nresult->chunk_id: %lu"
							"\nresult->decoder_state: %d"
							"\nresult->metadata_length: %u"
							"\nresult->min_block_size: %hu"
							"\nresult->max_block_size: %hu"
							"\nresult->min_frame_size: %u"
							"\nresult->max_frame_size: %u"
							"\nresult->channels: %d"
							"\nresult->hz: %ld"
							"\nresult->bps: %u"
							"\nresult->total_samples: %lu"
							"\nresult->encoding: %lu"
							"\nresult->is_last_metadata: %u"
							"\nresult->frame_bytes: %u"
							"\nresult->total_bytes_in_chunk: %lu"
							"\nresult->sample_size: %u"
							"\nresult->nsamples: %u"
							"\nresult->dec_input_chunk_ptr: %lu"
							"\nresult->dec_output_chunk_ptr: %lu"
							"\nresult->decoder_in_chunk_size: %lu"
							"\nresult->decoder_out_chunk_size: %lu\n";


#define FRAME_PRINT_PARAMETERS \
				MAXIMUM_SIZE_OGG_OUTPUT_BUFFER,\
							result->metadata_block_type,\
							result->chunk_id,\
							result->decoder_state,\
							result->metadata_length,\
							result->min_block_size,\
							result->max_block_size,\
							result->min_frame_size,\
							result->max_frame_size,\
							result->channels,\
							result->hz,\
							result->bps,\
							result->total_samples,\
							result->encoding,\
							result->is_last_metadata,\
							result->frame_bytes,\
							result->total_bytes_in_chunk,\
							result->sample_size,\
							result->nsamples,\
							result->dec_input_chunk_ptr,\
							result->dec_output_chunk_ptr,\
							result->decoder_in_chunk_size,\
							result->decoder_out_chunk_size

#define PRINT_FUNC

//NULL TERMINATED ALWAYS!
void print_string(const char* str){
	if(!stream_enable_ncurses){
		printf("%s",str);
	}
	else{
		pthread_mutex_lock(&ncurses_mtx);
		printw("%s",str);
		pthread_mutex_unlock(&ncurses_mtx);
	}

}
int should_switch(decoder_result_struct* before_result,decoder_result_struct* current_result){

	if(!before_result||!current_result){

		return -1;


	}
	return (before_result->channels!=current_result->channels)||(before_result->hz!=current_result->hz);




}
void print_decoder_frame_result(decoder_result_struct* result,int fd){

	if(result&&stream_show_frames){
		if(!stream_enable_ncurses){
			dprintf(fd,frame_print_format,FRAME_PRINT_PARAMETERS);
		}
		else{
			pthread_mutex_lock(&ncurses_mtx);
			printw(frame_print_format,FRAME_PRINT_PARAMETERS);
			pthread_mutex_unlock(&ncurses_mtx);
		}
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

int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* sound_buff_to_play,decoder_result_struct* result)
{
  int err;
  snd_pcm_sframes_t frames;
  frames = snd_pcm_writei(handle,sound_buff_to_play, result->total_bytes_in_chunk/result->sample_size/result->channels);
  if (frames < 0){
	frames = snd_pcm_recover(handle, frames, 0);
  }
  if ((err=frames) < 0) {
    	fprintf(stderr, "snd_pcm_writei failed to play %d bytes:\nerrno: %d\nerror string: %s\n",result->nsamples*result->channels*result->sample_size,err,strerror(errno));
	print_decoder_frame_result(result,1);
	return 1;
  }
  return 0;
}

int play_from_sound_device_oss(int oss_fd, uint8_t* buff,decoder_result_struct*result){

// Write PCM data to play audio
int written=-1;
if((written=write(oss_fd, buff, result->total_bytes_in_chunk))<0){
	fprintf(stderr,"There was an error writing to OSS sound device!!!\nAttempted to write %ld bytes to fd %d\nError: %s\n",result->total_bytes_in_chunk,oss_fd,strerror(errno));

}
return written;
}

int play_from_sound_device_pa(pa_simple* handle,uint8_t* sound_buff_to_play,decoder_result_struct* result)
{
    //if (pa_simple_write(handle, sound_buff_to_play, result->nsamples*result->channels*SIZE, NULL) < 0) {
    if (pa_simple_write(handle, sound_buff_to_play, result->total_bytes_in_chunk, NULL) < 0) {
        fprintf(stderr, "pa_simple_write() failed to play %d bytes:\nerrno: %d\nerror string: %s\n",result->nsamples*result->sample_size*result->channels,errno,pa_strerror(errno));
	print_decoder_frame_result(result,1);
	return 1;
    }

    return 0;
}
int play_from_sound_device_ao(ao_device *handle,uint8_t* buff_to_play,decoder_result_struct*result){

    //if (pa_simple_write(handle, sound_buff_to_play, result->nsamples*result->channels*SIZE, NULL) < 0) {
    if (ao_play(handle, (char*) buff_to_play, result->total_bytes_in_chunk) < 0) {
        fprintf(stderr, "ao_play() failed to play %d bytes:\nerrno: %d\nerror string: %s\n",result->nsamples*result->sample_size*result->channels,errno,strerror(errno));
	print_decoder_frame_result(result,1);
	return 1;
    }

    return 0;



}
