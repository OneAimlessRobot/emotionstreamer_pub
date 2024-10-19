#include "../../Includes/preprocessor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../Includes/ripped_code.h"



//https://discourse.libsdl.org/t/time-length-of-sdl-mixer-chunks/12852/2
/* untested code follows… */
uint32_t getChunkTimeMilliseconds(Mix_Chunk* chunk)
{
double points = 0.0;
double frames = 0.0;
int freq = 0;
uint16_t fmt = 0;
int chans = 0;
if (!Mix_QuerySpec(&freq, &fmt, &chans))
return 0;

 /* bytes / samplesize == sample points */
 points = ((double) chunk->alen / (double) ((fmt & 0xFF) / 8));

 /* sample points / channels == sample frames */
 frames = ((double) points / (double) chans);

 /* (sample frames * 1000) / frequency == play length in ms */
 return round((frames * 1000) / (double) freq);
}

