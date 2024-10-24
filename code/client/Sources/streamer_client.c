#include "../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/streamer_client.h"


static client_stream_t stream_struct={
					0,
					NULL,
					0,
					{0},
					NULL,
					NULL,
                                        NULL,
					0,
                                        };

static void zero_chk_cache(client_stream_t* strm){


        memset(strm->chunk_data_cache,0, cfg_chunk_size*cfg_stream_cache_size_chunks);


}

static void cleanALSA(client_stream_t* strm){

	if(strm->play_stream_alsa){

		snd_pcm_close(strm->play_stream_alsa);
	}

}
static void cleanPA(client_stream_t* strm){

	if(strm->play_stream_pa){

		pa_simple_flush(strm->play_stream_pa, NULL);
		pa_simple_free(strm->play_stream_pa);
	}

}
static void stop_client_stream(client_stream_t* strm){
	
	
	
	if(strm->innited){
       		close_con(strm->con_obj);
		strm->innited=0;
		switch(strm->which_mode){

			case PLAY_ALSA:
				cleanALSA(strm);
				break;
			case PLAY_PA:
				cleanPA(strm);
				break;
			default:
				break;

		}
		printf("SAIMOS DA STREAM DO CLIENTE! Vamos ver errno:%s\n",strerror(errno));
	}

}


static void sigint_handler(int useless){

	printf("SIGINT! ");
	stop_client_stream(&stream_struct + (0*useless));
	exit(useless);

}

static void sigpipe_handler(int useless){

	printf("SIGPIPE! ");
	sigint_handler(useless);

}
static void play_chunk_alsa(client_stream_t* strm){

	play_from_sound_device_alsa(strm->play_stream_alsa,(TYPE*)strm->chunk_data_cache,strm->curr_chk_index);
	zero_chk_cache(strm);
	strm->curr_chk_index=0;
}
static void play_chunk_pa(client_stream_t* strm){

	play_from_sound_device_pa(strm->play_stream_pa,(TYPE*)strm->chunk_data_cache,strm->curr_chk_index);
	zero_chk_cache(strm);
	strm->curr_chk_index=0;
}

static void play_chunk(client_stream_t* strm){
       		switch(strm->which_mode){

			case PLAY_ALSA:
				play_chunk_alsa(strm);
				break;
			case PLAY_PA:
				play_chunk_pa(strm);
				break;
			default:
				break;

		}

}
static int process_chunk(client_stream_t* strm){


	int wait=0;
	strm->curr_chk_index+=cfg_chunk_size;
	if((strm->curr_chk_index)==(cfg_chunk_size*cfg_stream_cache_size_chunks)){
		wait=1;

	}
        return wait;
}

static int read_chunk_tcp(client_stream_t* strm,int_pair pair){

	 return readsome(strm->con_obj->sockfd_tcp,(char*)(strm->chunk_data_cache+strm->curr_chk_index),cfg_chunk_size,pair);

}
static int read_chunk_udp(client_stream_t* strm,int_pair pair){

        return readsome_udp(strm->con_obj->sockfd_udp,(char*)(strm->chunk_data_cache+strm->curr_chk_index),cfg_chunk_size,pair,&strm->con_obj->peer_udp_addr);
}


int get_server_chunk(client_stream_t* strm,int_pair pair){

        int result= read_chunk_udp(strm, pair);
	if(result<0){

                return result;
	}
	int wait=process_chunk(strm);
	snprintf((char*)strm->con_obj->ack_udp_data,cfg_datasize,"%s %d",ACK_STRING,wait);
	result=con_send_udp_ack(strm->con_obj,pair);
        if(result<0){

                return result;
	}
	if(wait){
		play_chunk(strm);
		clear_con_data(strm->con_obj);
		snprintf((char*)strm->con_obj->ack_udp_data,cfg_datasize-1,"%s",CON_STRING);
		result= con_send_udp_ack(strm->con_obj,client_data_times_pair);
	}
	return result;

}

static void initALSA(client_stream_t* strm){

int err;
if ((err=snd_pcm_open(&strm->play_stream_alsa, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0){
     printf("Playback open error: %s\n", snd_strerror(err));
     exit(-1);
}

	strm->innited=1;

if ((err =snd_pcm_set_params(strm->play_stream_alsa,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,CHANNELS,cfg_freq, 1, cfg_latency_us) ) < 0 ){
	        printf("Playback open error: %s\n", snd_strerror(err));
 		raise(SIGINT);
	}


}

static void client_stream(client_stream_t* strm){

        while(strm->con_obj->is_on&&(get_server_chunk(strm,client_data_times_pair)>0)){}
	process_chunk(strm);
	play_chunk(strm);
}
static void initPA(client_stream_t*strm){
     pa_sample_spec ss = {
         .format = PA_SAMPLE_S16LE,
         .rate = cfg_freq,
         .channels = CHANNELS
     };

     if (!(strm->play_stream_pa = pa_simple_new(NULL, "client.exe", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &errno))) {
         fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(errno));
         return;
	}
	strm->innited=1;

}
static int init_client_stream(client_stream_t* strm,con_t* con_obj, unsigned char* buff,method which_mode)
{
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	strm->which_mode=which_mode;
       	switch(strm->which_mode){

		case PLAY_ALSA:
			initALSA(strm);
			break;
		case PLAY_PA:
			initPA(strm);
			break;
		default:
			break;

	}
	
	strm->chunk_data_cache=buff;
	memset(strm->chunk_data_cache,0,cfg_chunk_size*cfg_stream_cache_size_chunks);
	strm->con_obj=con_obj;
        client_stream(strm);
        raise(SIGINT);
	return 0;
}



void player_stop_stream(void){
	raise(SIGINT);
}

void player_init_stream(con_t* con_obj,unsigned char* buff,method which_mode){

	init_client_stream(&stream_struct,con_obj,buff,which_mode);
}

