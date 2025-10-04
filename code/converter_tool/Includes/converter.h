#ifndef CONVERTER_H
#define CONVERTER_H

#define BOUNDARY_FILE_EXT ".boundary"

//AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
/* Index: 0–15.  0 = free, 15 = bad/invalid. */
static const int bitrate_table[4][3][16] = {
/* MPEG Version 2 & 2.5 (same table) */
{
    /* Layer I   */ {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
    /* Layer II  */ {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
    /* Layer III */ {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}
},
{
	{0},
	{0},
	{0}
},
/* MPEG Version 2 & 2.5 (same table) */
{
    /* Layer I   */ {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
    /* Layer II  */ {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
    /* Layer III */ {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}
},
/* MPEG Version 1 */
{
    /* Layer I   */ {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
    /* Layer II  */ {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,0},
    /* Layer III */ {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
}

};

static const int samplerate_table[4][3] = {
/* index: 00, 01, 10, 11(reserved) */
    {11025,12000,8000},   // MPEG-2.5
    {0,0,0},               // reserved
    {22050,24000,16000},  // MPEG-2
    {44100,48000,32000} // MPEG-1
};

#define MP3_WORD_BITS 32
#define MP3_SAMPLE_SIZE (MP3_WORD_BITS/8)
#define SYNC_BITS_MASK            0b11111111111000000000000000000000
#define VERSION_BITS_MASK         0b00000000000110000000000000000000
#define LAYER_BITS_MASK           0b00000000000001100000000000000000
#define PROTECTION_BITS_MASK      0b00000000000000010000000000000000
#define BITRATE_BITS_MASK         0b00000000000000001111000000000000
#define SAMPLE_RATE_BITS_MASK     0b00000000000000000000110000000000
#define PADDING_BIT_MASK          0b00000000000000000000001000000000
#define PRIVATE_BIT_MASK          0b00000000000000000000000100000000
#define CHANNEL_MODE_BITS_MASK    0b00000000000000000000000011000000
#define MODE_EXTENSION_BITS_MASK  0b00000000000000000000000000110000
#define COPYRIGHT_BIT_MASK        0b00000000000000000000000000001000
#define ORIGINAL_BIT_MASK         0b00000000000000000000000000000100
#define EMPHASIS_BITS_MASK        0b00000000000000000000000000000011

#define MP3_VERSION_SHIFT      19
#define MP3_LAYER_SHIFT        17
#define MP3_PROTECTION_SHIFT   16
#define MP3_BITRATE_SHIFT      12
#define MP3_SAMPLERATE_SHIFT   10
#define MP3_PADDING_SHIFT      9
#define MP3_PRIVATE_SHIFT      8
#define MP3_CHANNELMODE_SHIFT  6
#define MP3_MODEEXT_SHIFT      4
#define MP3_COPYRIGHT_SHIFT    3
#define MP3_ORIGINAL_SHIFT     2
#define MP3_EMPHASIS_SHIFT     0

#define INV_SHIFT(a) (MP3_WORD_BITS-(a)-2);


//MPEG version
#define         MPEG25  0
#define         MPEG1   3
#define         MPEG2   2






typedef struct frame_info_t{

	uint32_t start,size;


}frame_info_t;


typedef struct frame_info_machine{

	uint64_t n_frames;

	int fd_in,fd_out;


}frame_info_machine_t;



void start_frame_info_machine(const char* file_name_in,const char* file_name_out,int dry_run);


void end_frame_info_machine(frame_info_machine_t* machine);

#endif
