#include "../../Includes/preprocessor.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include <sys/ioctl.h> //for ioctl()
#include <linux/soundcard.h> //SOUND_PCM*
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include <alsa/asoundlib.h>
#include "../Includes/ripped_code.h"


/*
static void print_chunk_helper_info(chunk_size_helper* helper){
	printf("Info deste helper: Frequencia: %d\nChannels: %d\nFormato: %u\nAudio len %u\n",helper->freq,helper->chans,helper->fmt,helper->audio_len);

}
*/

//https://discourse.libsdl.org/t/time-length-of-sdl-mixer-chunks/12852/2
/* untested code follows… */

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
 
int play_from_sound_device_alsa(snd_pcm_t* handle,uint8_t* sound_buff_to_play,int size)
{
  int err;
  snd_pcm_sframes_t frames;
  int samples= size/SIZE;
  frames = snd_pcm_writei(handle,sound_buff_to_play, samples/CHANNELS);
  if (frames < 0){
	frames = snd_pcm_recover(handle, frames, 0);
  }
  if ((err=frames) < 0) {
    	printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
  }
  return 0;
}

int play_from_sound_device_pa(pa_simple* handle,uint8_t* sound_buff_to_play,int size)
{
    if (pa_simple_write(handle, sound_buff_to_play, size, NULL) < 0) {
        fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(errno));
        return 1;
    }

    return 0;
}



//https://digitalsoundandmusic.com/5-3-2-raw-audio-data-in-c/
//This program runs under OSS
//The program expects an 8-bit raw sound file.
//You can alter it to read a 16-bit file into short ints

/*
static void writeToSoundDevice(TYPE* buf, int deviceID, int buffSize) {
  int status;
  status = write(deviceID, buf, buffSize);
  if (status != buffSize)
    perror("Wrote wrong number of bytes\n");
  status = ioctl(deviceID, SOUND_PCM_SYNC, 0);
  if (status == -1)
    perror("SOUND_PCM_SYNC failed\n");
}
*/
/* 
static int play_from_sound_device(TYPE sound_buff_to_play[])
{
  int deviceID, arg, status, i;
  deviceID = open("/dev/snd/pcmC0D2c", O_WRONLY, 0);
  if (deviceID < 0)
    perror("Opening /dev/snd/pcmC0D2c failed\n");
  arg = SIZE * 8;
  status = ioctl(deviceID, SOUND_PCM_WRITE_BITS, &arg);
  if (status == -1)
    perror("Unable to set sample size\n");
  arg = CHANNELS;
  status = ioctl(deviceID, SOUND_PCM_WRITE_CHANNELS, &arg);
  if (status == -1)
    perror("Unable to set number of channels\n");
  status = ioctl(deviceID, SOUND_PCM_WRITE_RATE, &cfg_freq);
  if (status == -1)
    perror("Unable to set number of bits\n");
 
  writeToSoundDevice(sound_buff_to_play, deviceID, cfg_chunk_size);
  close(deviceID);
}
*/
