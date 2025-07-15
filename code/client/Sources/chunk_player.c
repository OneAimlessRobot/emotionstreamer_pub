#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include "../../wav_stuff_i_stole_because_i_am_lazy/wav.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/ogg_module.h"
#include "../Includes/chunk_player.h"

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static int wav_header_received=0;
static void parse_wav_header_into_player_result(chunk_player* player){


	WAVFile file=WAV_ParseFileData(player->h_chunk);
	player->current_result.channels=file.header.number_of_channels;
	player->current_result.hz=file.header.sample_rate;
	player->current_result.bps=file.header.bits_per_sample;
	player->current_result.sample_size=(((int)ceil((((float)player->current_result.bps)/8.0f))));
	player->current_result.total_bytes_in_chunk=player->chunk_size;
	print_decoder_frame_result(&player->current_result,1);
}

static void cleanALSA(chunk_player* player){

	if(player->play_stream_alsa){

		snd_pcm_close(player->play_stream_alsa);
	}

}
static void cleanPA(chunk_player* player){

	if(player->play_stream_pa){

		pa_simple_flush(player->play_stream_pa, NULL);
		pa_simple_free(player->play_stream_pa);
	}

}
static void clean_player(chunk_player* player){
		switch(player->which_mode){

			case PLAY_ALSA:
				cleanALSA(player);
				break;
			case PLAY_PA:
				cleanPA(player);
				break;
			default:
				break;
		}

}



static void initALSA(chunk_player* player){

int err;
if ((err=snd_pcm_open(&player->play_stream_alsa, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0){
     printf("Playback open error: %s\n", snd_strerror(err));
     exit(-1);
}

if ((err =snd_pcm_set_params(player->play_stream_alsa,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,player->current_result.channels,player->current_result.hz, 1, cfg_latency_ms*1000) ) < 0 ){
	        printf("Playback open error: %s\n", snd_strerror(err));
 		raise(SIGINT);
		abort();
	}
	else{
		printf("ALSA initialized successfully!!!!\n");
	}

}

static void initPA(chunk_player*player){
     pa_sample_spec ss = {
         .format = PA_SAMPLE_S16LE,
         .rate = player->current_result.hz,
         .channels = player->current_result.channels
     };

     if (!(player->play_stream_pa = pa_simple_new(NULL, "client.exe", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &errno))) {
         fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(errno));
         raise(SIGINT);
	 abort();
	}
	else{

	printf("pulseaudio initialized successfully!!!!\n");

	}
}

static void init_player_lib(chunk_player* player){

	int should_initialize=0;
	if(is_wav_mode&&!wav_header_received){

		parse_wav_header_into_player_result(player);
		wav_header_received=1;
		should_initialize=1;
	}
	else if(!wav_header_received){

		should_initialize=1;
	}
	if(should_initialize){
		switch(player->which_mode){

			case PLAY_ALSA:
				initALSA(player);
				break;
			case PLAY_PA:
				initPA(player);
				break;
			default:
				break;

		}
	}
}
static void play_chunk_alsa(chunk_player* player){
	play_from_sound_device_alsa(player->play_stream_alsa,player->p_chunk,&player->current_result);

}
static void play_chunk_pa(chunk_player* player){
	play_from_sound_device_pa(player->play_stream_pa,player->p_chunk,&player->current_result);
}

static void play_chunk(chunk_player* player,int dry){

		if(!dry){


		switch(player->which_mode){

			case PLAY_ALSA:
				play_chunk_alsa(player);
				break;
			case PLAY_PA:
				play_chunk_pa(player);
				break;
			default:
				break;

		}
	}
}

static void write_player_result(chunk_player* player,decoder_result_struct* result,int in){

	if(result){
		if(in){
			memcpy(&player->current_result,result,sizeof(decoder_result_struct));
		}
		else {
			memcpy(result,&player->current_result,sizeof(decoder_result_struct));
		}
		
	}
}
static void print_player_result(chunk_player* player){
	print_decoder_frame_result(&player->current_result,1);
}
static void safe_play_wrapper(chunk_player* player,decoder_result_struct* result,int dry){
	
	if(result&&result->total_bytes_in_chunk){
		if((should_switch(result,&player->current_result)!=0)&&!is_wav_mode){
			clean_player(player);
			write_player_result(player,result,1);
			init_player_lib(player);
		}

		play_chunk(player,dry);
	}
	else if(is_wav_mode){

		play_chunk(player,dry);


	}
}
void perform_play_op(chunk_player* player,decoder_result_struct* result,play_op op){
	switch(op){
		case P_REAL_PLAY:
			safe_play_wrapper(player,result,0);
			break;
		case P_DRY_PLAY:
			safe_play_wrapper(player,result,1);
			break;
		case P_GET_FRAME_DATA:
			write_player_result(player,result,0);
			break;
		case P_INSERT_FRAME_DATA:
			write_player_result(player,result,1);
			break;
		case P_PRINT_FRAME_DATA:
			print_player_result(player);
			break;
		case P_CLEAN:
			clean_player(player);
			break;
		case P_INIT_LIBS:
			init_player_lib(player);
			break;
		case P_PLAY_NA:
			break;
		case P_PARSE_WAV_HEADER_INTO_PLAYER_RESULT:
			parse_wav_header_into_player_result(player);
			break;
		default:
			break;
	}
}

int init_chunk_player(chunk_player* player,uint64_t chunk_size,uint8_t* h_buff,uint8_t* r_buff,uint8_t* p_buff,method the_way){
	player->mtx=&mtx;
	player->chunk_size=chunk_size;
	player->p_chunk=p_buff;
	player->r_chunk=r_buff;
	player->h_chunk=h_buff;
	player->which_mode=the_way;
	memset(&player->current_result,0,sizeof(decoder_result_struct));
	return 0;

}
