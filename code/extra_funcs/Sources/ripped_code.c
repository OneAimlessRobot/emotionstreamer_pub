#include "../../Includes/preprocessor.h"
#include "../Includes/ripped_code.h"



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
 return round((frames * 1000) / (double) helper->freq);
}

