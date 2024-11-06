#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_player.h"

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;


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

if ((err =snd_pcm_set_params(player->play_stream_alsa,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,CHANNELS,cfg_freq, 1, cfg_latency_ms*1000) ) < 0 ){
	        printf("Playback open error: %s\n", snd_strerror(err));
 		raise(SIGINT);
	}


}

static void initPA(chunk_player*player){
     pa_sample_spec ss = {
         .format = PA_SAMPLE_S16LE,
         .rate = cfg_freq,
         .channels = CHANNELS
     };

     if (!(player->play_stream_pa = pa_simple_new(NULL, "client.exe", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &errno))) {
         fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(errno));
         return;
	}

}


static void play_chunk_alsa(chunk_player* player
){

	play_from_sound_device_alsa(player->play_stream_alsa,player->p_chunk,(player->chunk_size));
}
static void play_chunk_pa(chunk_player* player){
	play_from_sound_device_pa(player->play_stream_pa,player->p_chunk,(player->chunk_size));
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

static void swap_buffs(chunk_player* player){
	
	pthread_mutex_lock(player->mtx);
	uint8_t* tmp=player->p_chunk;
	player->p_chunk=player->r_chunk;
	player->r_chunk=tmp;
	pthread_mutex_unlock(player->mtx);




}

//BUFF IS PLAYER->CHUNK_SIZE in SIZE!!!
static void write_to_buff(chunk_player* player, uint8_t* buff){

	memcpy(player->r_chunk,buff,player->chunk_size);

}
void perform_play_op(chunk_player* player,uint8_t* buff,play_op op){

	switch(op){
		case P_REAL_PLAY:
			play_chunk(player,0);
			break;
		case P_DRY_PLAY:
			play_chunk(player,1);
			break;
		case P_WRITE_TO_BUFF:
			write_to_buff(player,buff);
			break;
		case P_SWAP:
			swap_buffs(player);
			break;
		case P_CLEAN:
			clean_player(player);
			break;
		case P_PLAY_NA:
			break;
		default:
			break;
	}
}
int init_chunk_player(chunk_player* player,uint16_t chunk_size,uint8_t* p_buff,uint8_t* r_buff,method the_way){
	player->mtx=&mtx;
	player->chunk_size=chunk_size;
	player->p_chunk=p_buff;
	player->r_chunk=r_buff;
	player->which_mode=the_way;
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
	return 0;

}
