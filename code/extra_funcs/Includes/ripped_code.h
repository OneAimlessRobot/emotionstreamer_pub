#ifndef RIPPED_CODE_H
#define RIPPED_CODE_H

typedef struct chunk_size_helper{
	int32_t freq,chans;
	uint16_t fmt;
	uint32_t audio_len;


}chunk_size_helper;



uint32_t getChunkTimeMilliseconds(chunk_size_helper* helper);

#endif
