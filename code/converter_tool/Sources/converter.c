#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/converter.h"
#include "../Includes/configs.h"



static frame_info_machine_t machine={0};
static int get_mp3_frame_size(int version, int layer, int bitrate, int samplerate, int padding) {
    if (bitrate == 0 || samplerate == 0)
        return 0; // invalid

    if (layer == 1)
        return ((12 * bitrate / samplerate) + padding) * 4;
    else if (version == 3) // MPEG1
        return (144 * bitrate / samplerate) + padding;
    else                    // MPEG2 / 2.5
        return (72 * bitrate / samplerate) + padding;
}

static void sigint_handler(int signal_arg){

	fprintf(stderr,"Sigint called!!!\n%s\n",strerror(errno+(signal_arg*0)));
	end_frame_info_machine(&machine);
	exit(-1);

}

void start_frame_info_machine(const char* file_name_in, const char* file_name_out,int dry_run){
	if(!file_name_in){

		fprintf(stderr,"Null input file name string!\n%s\n",strerror(errno));
		return;
	}
	if(!file_name_out){

		fprintf(stderr,"Null output file name string!\n%s\n",strerror(errno));
		return;
	}

	char path_buff_in[PATHSIZE*2]={0};
	char path_buff_out[PATHSIZE*10]={0};
	snprintf(path_buff_in,sizeof(path_buff_in)-1,"%s/%s",converter_in_dir,file_name_in);
	snprintf(path_buff_out,sizeof(path_buff_out)-1,"%s/%s%s",converter_out_dir,file_name_out,BOUNDARY_FILE_EXT);

	machine.fd_in=open(path_buff_in,O_RDONLY,0777);
	if(machine.fd_in<0){

		fprintf(stderr,"Error at opening frame_info_machine input fd!: %s\nPath: %s\n",strerror(errno),path_buff_in);
		return;
	}
	machine.fd_out=(dry_run?0:open(path_buff_out,O_WRONLY|O_TRUNC|O_CREAT,0777));
	if(machine.fd_out<0){

		fprintf(stderr,"Error at opening frame_info_machine output fd!: %s\nPath: %s\n",strerror(errno),path_buff_out);
		close(machine.fd_in);
		return;
	}
	signal(SIGINT,sigint_handler);
	int nread=-1;
	uint64_t curr_ftell=0;
	int nwritten=-1;
	char curr_sample[MP3_SAMPLE_SIZE]={0};
	while((nread=read(machine.fd_in,curr_sample,MP3_SAMPLE_SIZE)==4)){
		if ((((uint32_t)curr_sample[0])&SYNC_BITS_MASK)==SYNC_BITS_MASK)
		{
		uint32_t hdr = (curr_sample[0] << 24) | (curr_sample[1]  << 16) | (curr_sample[2]  << 8) | curr_sample[3];

		//printf("We found a frame header!!!\n");
		uint32_t mpeg_layer	             = (hdr & LAYER_BITS_MASK)        >> MP3_LAYER_SHIFT;
		if(mpeg_layer!=1){
			//printf("....But its [not] layer 3. So we scrap it\n");
			continue;
		}
		//usleep(1000000);
		uint32_t mpeg_version	     = ((uint32_t)(hdr & VERSION_BITS_MASK))      >> MP3_VERSION_SHIFT;
		uint32_t mpeg_bitrate_idx 	     = ((uint32_t)(hdr & BITRATE_BITS_MASK))     >> MP3_BITRATE_SHIFT;
		uint32_t mpeg_sample_rate_idx     = ((uint32_t)(hdr & SAMPLE_RATE_BITS_MASK))  >> MP3_SAMPLERATE_SHIFT;
		uint32_t mpeg_padding     	     = ((uint32_t)(hdr & PADDING_BIT_MASK))       >> MP3_PADDING_SHIFT;
		uint32_t mpeg_channelmode 	     = ((uint32_t)(hdr & CHANNEL_MODE_BITS_MASK)) >> MP3_CHANNELMODE_SHIFT;
		if(mpeg_sample_rate_idx==3){

			//printf("Invalid sample_rate idx.\nSkipping...\n");
			continue;

		}
		int sample_rate=samplerate_table[mpeg_version][mpeg_sample_rate_idx];
		int bitrate=bitrate_table[mpeg_version][mpeg_layer][mpeg_bitrate_idx]*1000;
		curr_ftell=lseek(machine.fd_in,0,SEEK_CUR);
		frame_info_t frame_info_struct={curr_ftell-4,0};
		if(!(frame_info_struct.size=get_mp3_frame_size(mpeg_version,mpeg_layer,bitrate,sample_rate,mpeg_padding))){
			continue;
		}

		/*printf("This frame header has the following data:\n"
						"mpeg_layer: %u\n"
						"mpeg_version: %u\n"
						"mpeg_bitrate_idx: %u\n"
						"mpeg_sample_rate_idx: %u\n"
						"bitrate: %d\n"
						"sample_rate: %d\n"
						"mpeg_padding: %u\n"
						"mpeg_channelmode: %u\n"
						"frame_info_struct.start: %lu\n"
						"frame_info_struct.size: %lu\n\n\n",
						mpeg_layer,
						mpeg_version,
						mpeg_bitrate_idx,
						mpeg_sample_rate_idx,
						bitrate,
						sample_rate,
						mpeg_padding,
						mpeg_channelmode,
						frame_info_struct.start,
						frame_info_struct.size
						);
		*/
		if(!dry_run){
			if((nwritten=write(machine.fd_out,&frame_info_struct,sizeof(frame_info_struct)))<0){
				fprintf(stderr,"We had issues with writing frame header info to file!\nAborting!\n%s\n",strerror(errno));
				end_frame_info_machine(&machine);
				return;
			}
		}
		}
		memset(curr_sample,0,sizeof(curr_sample));
	}
	printf("Frame machine completed task!\n");
	end_frame_info_machine(&machine);
}



void end_frame_info_machine(frame_info_machine_t* machine){

	printf("Exiting from frame machine\n");
	close(machine->fd_in);
	close(machine->fd_out);


}


