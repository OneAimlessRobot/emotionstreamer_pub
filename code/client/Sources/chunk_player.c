#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include "../../wav_stuff_i_stole_because_i_am_lazy/wav.h"
#include <ao/ao.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <pulse/xmalloc.h>
#include <pulse/error.h>
#include <pulse/stream.h>
#include <pulse/proplist.h>

struct pa_simple {
    pa_threaded_mainloop *mainloop;
    pa_context *context;
    pa_stream *stream;
    pa_stream_direction_t direction;

    const void *read_data;
    size_t read_index, read_length;

    int operation_success;
};

#include <pulse/simple.h>
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/mp3module.h"
#include "../Includes/chunk_player.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/streamer_client.h"
#include <linux/soundcard.h>
#include <sys/ioctl.h>

// frag <- ((upper_sixteen_bits << 16) | lower_sixteen_bits)
//it will play the same chunk priming_chunks number of times before actual startup
typedef struct oss_frag_params{
	uint32_t upper_sixteen_bits,
		lower_sixteen_bits,
		is_priming_set,
		priming_chunks;

}oss_frag_params;

static oss_frag_params main_oss_params={1,13,0,5};

#define FRAG_PARAM_FOR_OSS ((main_oss_params.upper_sixteen_bits << 16) | main_oss_params.lower_sixteen_bits)

#define OSS_DEVICE_NAME "/dev/dsp"

static pa_sample_spec ss={0};

static ao_sample_format format={0};

static char tmp_dev_string[DEF_DATASIZE]={0},
	/*number_string_for_oss_ao_plugin_buffer_time[DEF_DATASIZE]={0},*/
	alsa_device_print_buff[DEF_DATASIZE]={0};

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

static int wav_header_received=0,
	driver_id=-1,
	innited=0;

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

static void cleanAO(chunk_player* player){

	if(player->play_stream_ao){

	    ao_close(player->play_stream_ao);
	    ao_shutdown();

	}


}

static void cleanOSS(chunk_player*player){

	if((player->oss_sound_fd>0)){

		close(player->oss_sound_fd);

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
			case PLAY_AO:
				cleanAO(player);
				break;
			case PLAY_BARE:
				cleanOSS(player);
				break;
			default:
				break;
		}

}
static void initOSS(chunk_player* player){

// Example: 16-bit, 44100 Hz, stereo

if(player->oss_sound_fd>0){
	close(player->oss_sound_fd);
	player->oss_sound_fd=-1;
}
player->oss_sound_fd = open(OSS_DEVICE_NAME, O_WRONLY);
if (player->oss_sound_fd < 0) {
    fprintf(stderr,"Could not open OSS sound file descriptor!!!\nError: %s\n",strerror(errno));
    	raise(SIGINT);
	stop_client_stream();
	return;
}

printf("Innited OSS file descriptor... All that is left is setting ioctl opts!\n");
int frag= FRAG_PARAM_FOR_OSS;
ioctl(player->oss_sound_fd, SNDCTL_DSP_SETFRAGMENT, &frag);

int format = AFMT_S16_LE;

ioctl(player->oss_sound_fd, SNDCTL_DSP_SETFMT, &format);


ioctl(player->oss_sound_fd, SNDCTL_DSP_CHANNELS, &player->current_result.channels);

ioctl(player->oss_sound_fd, SNDCTL_DSP_SPEED, &player->current_result.hz);

printf("... Which we just did!...\n[thumbs up]\nWe're done we play sound wedoeht!\n");



}

//https://stackoverflow.com/questions/6866103/finding-all-the-devices-i-can-use-to-play-pcm-with-alsa

static void list_alsa_devices(void){

char **hints;
/* Enumerate sound devices */
int err = snd_device_name_hint(-1, "pcm", (void***)&hints);
if (err != 0)
   return;//Error! Just return

char** n = hints;
char* curr_ptr_in_buff=alsa_device_print_buff;
memset(alsa_device_print_buff,0,sizeof(alsa_device_print_buff));
curr_ptr_in_buff+=snprintf(alsa_device_print_buff,sizeof(alsa_device_print_buff)-1,"Behold... Device names for ALSA!\n(For ALSA use only configs not included)\n\n");
int count=0;
while (*n != NULL) {

    char *name = snd_device_name_get_hint(*n, "NAME");

    if (name != NULL && 0 != strcmp("null", name)) {
	curr_ptr_in_buff+=snprintf(curr_ptr_in_buff,sizeof(alsa_device_print_buff)-1,"\n%d- Device name: %s\n",count,name);
	count++;
    }
    n++;
    if(name){
	free(name);
    }
}//End of while

//Free hint buffer too
snd_device_name_free_hint((void**)hints);
print_string(alsa_device_print_buff);


}

static void initALSA(chunk_player* player){

list_alsa_devices();
int err;
memset(tmp_dev_string,0,sizeof(tmp_dev_string));
snprintf(tmp_dev_string,strlen(cfg_client_device_name_if_alsa)+strlen(cfg_client_device_output_if_alsa)+10,"%s,%s",cfg_client_device_name_if_alsa,cfg_client_device_output_if_alsa);

if(!innited){
	if ((err=snd_pcm_open(&player->play_stream_alsa, tmp_dev_string, SND_PCM_STREAM_PLAYBACK, 0)) < 0){
	     printf("Playback open error: device name obtained: %s\nError: %s\n",tmp_dev_string, snd_strerror(err));
	     raise(SIGINT);
	     stop_client_stream();
	     return;
	}

}
if ((err =snd_pcm_set_params(player->play_stream_alsa,
					SND_PCM_FORMAT_S16_LE,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					player->current_result.channels,
					player->current_result.hz,
					1,
					MS_TO_US(cfg_client_alsa_device_latency_if_alsa_ms)) ) < 0 ){


		printf("Playback open error: %s\n", snd_strerror(err));
 		raise(SIGINT);
		stop_client_stream();
		return;
	}
	else{
		printf("ALSA initialized successfully!!!!\n");
	}

}
static void print_driver_infos(void){

	int count=0;
	ao_info**infos=ao_driver_info_list(&count);
	for(int i=0;i<count;i++){
		printf("We found %d drivers.\n"
					"Those are the following:\n\n\n"
					"%d:	Name: %s\n\n"
					"	Type: %s\n\n"
					"	short_name: %s\n\n"
					"	comment: %s\n\n"
					"	prefered_byte_format: %d\n\n"
					"	priority: %d\n\n",
					count,
					i,
					infos[i]->name,
					(infos[i]->type==AO_TYPE_LIVE)?"Live!":"Output...",
					infos[i]->short_name,
					infos[i]->comment,
					infos[i]->preferred_byte_format,
					infos[i]->priority);
		printf("\n\n\nThis driver has %d options.\nThey are the following:\n",infos[i]->option_count);
		for(int j=0;j<infos[i]->option_count;j++){
			printf("Option %d: %s\n",j,infos[i]->options[j]);
		}
	}

}
static void initPA(chunk_player*player){
     memset(&ss,0,sizeof(pa_sample_spec));
     ss.format =PA_SAMPLE_S16LE;
     ss.rate = player->current_result.hz;
     ss.channels = player->current_result.channels;
     if (!(player->play_stream_pa = pa_simple_new(NULL, "client.exe", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &errno))) {
         fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(errno));
        raise(SIGINT);
	stop_client_stream();
	return;
	}
	else{

	printf("pulseaudio initialized successfully!!!!\n");

	}
}

static void changePA(chunk_player*player){
     memset(&ss,0,sizeof(pa_sample_spec));
     ss.format =PA_SAMPLE_S16LE;
     ss.rate = player->current_result.hz;
     ss.channels = player->current_result.channels;
     if(player->play_stream_pa->stream){

	pa_proplist* proplist=pa_proplist_new();;
	pa_proplist_set(proplist, PA_PROP_FORMAT_RATE , (int*)&(ss.rate), 4);
	pa_proplist_set(proplist, PA_PROP_FORMAT_CHANNELS  , (int*)&(ss.channels), 4);
	pa_stream_proplist_update(player->play_stream_pa->stream, PA_UPDATE_REPLACE, proplist,NULL,NULL);
     	pa_proplist_free(proplist);
	printf("pulseaudio changed successfully!!!!\n");
     }
}

static void initAO(chunk_player*player){
	memset(&format,0,sizeof(ao_sample_format));
	format.byte_format = AO_FMT_NATIVE;
	format.rate = player->current_result.hz;
	format.bits = 16;
        ao_option* options=NULL;

	print_driver_infos();
	driver_id = ao_driver_id("pulse");
	if(driver_id<0){
		printf("Error opening libao sound driver.\n");
		raise(SIGINT);
		stop_client_stream();
		return;
	}
	player->play_stream_ao = ao_open_live(driver_id, &format, options);
	if (player->play_stream_ao == NULL) {
		fprintf(stderr, "Error opening libao sound device from driver id %d\nError number: %d\nError string:\n",driver_id, errno);
		switch(errno){
			case AO_ENODRIVER:
				fprintf(stderr,"No driver corresponds to driver_id.\n");
				break;
			case AO_ENOTLIVE:
				fprintf(stderr,"This driver is not a live output device.\n");
				break;
			case AO_EBADOPTION:
				fprintf(stderr,"A valid option key has an invalid value.\n");
				break;
			case AO_EOPENDEVICE:
				fprintf(stderr,"Cannot open the device\n");
				break;
			case AO_EFAIL:
				fprintf(stderr,"Unspecified error.\n");
				break;
			default:
				break;

		}
		raise(SIGINT);
		stop_client_stream();
		return;
	}
	else{

		printf("libao initialized successfully!!!!\nThe driver id is %d\n",driver_id);

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
				if(!innited){
					initALSA(player);
				}
				break;
			case PLAY_PA:
				if(!innited){
					initPA(player);
				}
				else{
					changePA(player);
				}
				break;
			case PLAY_AO:
				if(!innited){
					initAO(player);
				}
				break;
			case PLAY_BARE:
				if(!innited){
					initOSS(player);
				}
				break;
			default:
				break;

		}
	}
}
static void play_chunk_oss(chunk_player* player){
	play_from_sound_device_oss(player->oss_sound_fd,player->pr_chunk,&player->current_result);
}
static void play_chunk_alsa(chunk_player* player){
	play_from_sound_device_alsa(player->play_stream_alsa,player->pr_chunk,&player->current_result);

}
static void play_chunk_pa(chunk_player* player){
	play_from_sound_device_pa(player->play_stream_pa,player->pr_chunk,&player->current_result);
}

static void play_chunk_ao(chunk_player* player){
	play_from_sound_device_ao(player->play_stream_ao,player->pr_chunk,&player->current_result);
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
			case PLAY_AO:
				play_chunk_ao(player);
				break;
			case PLAY_BARE:
				play_chunk_oss(player);
				break;
			default:
				break;

		}
	}
}

static void write_player_result(chunk_player* player,decoder_result_struct* result,int in){

	if(in){
		memcpy(&player->current_result,result,sizeof(decoder_result_struct));
	}
	else {
		memcpy(result,&player->current_result,sizeof(decoder_result_struct));
	}
}
static void print_player_result(chunk_player* player){
	print_decoder_frame_result(&player->current_result,1);
}
static void safe_play_wrapper(chunk_player* player,int dry){
	decoder_result_struct* result=&((mp3_processed_chunk*)player->p_chunk)->result_struct;
	if(result&&result->total_bytes_in_chunk){
		if((should_switch(result,&player->current_result)!=0)&&!is_wav_mode){
			write_player_result(player,result,1);
			init_player_lib(player);
			innited=1;
		}
		else{
			play_chunk(player,dry);
		}
	}
	else if(is_wav_mode){

		play_chunk(player,dry);


	}
}
static void safe_decoder_buff_load(chunk_player* player){

	memcpy(player->pr_chunk,player->p_chunk+sizeof(decoder_result_struct)+4,((mp3_processed_chunk*)player->p_chunk)->result_struct.total_bytes_in_chunk);

}
void perform_play_op(chunk_player* player,decoder_result_struct* result,play_op op){
	pthread_mutex_lock(&mtx);
	switch(op){
		case P_SAFE_DECODER_BUFF_LOAD:
			safe_decoder_buff_load(player);
			break;
		case P_REAL_PLAY:
			safe_play_wrapper(player,0);
			break;
		case P_DRY_PLAY:
			safe_play_wrapper(player,1);
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
	pthread_mutex_unlock(&mtx);
}

int init_chunk_player(chunk_player* player,uint64_t chunk_size,uint64_t pr_chunk_size,uint8_t* h_buff,uint8_t* r_buff,uint8_t* p_buff,uint8_t* pr_chunk,method the_way){
	player->mtx=&mtx;
	player->chunk_size=chunk_size;
	player->pr_chunk_size=pr_chunk_size;
	player->p_chunk=p_buff;
	player->r_chunk=r_buff;
	player->h_chunk=h_buff;
	player->pr_chunk=pr_chunk;
	player->which_mode=the_way;
	memset(&player->current_result,0,sizeof(decoder_result_struct));
	return 0;

}
