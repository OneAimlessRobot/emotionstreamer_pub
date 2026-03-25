#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/converter.h"
#include "../Includes/configs.h"



static frame_info_machine_t machine={0};
static uint16_t get_mp3_frame_size(uint16_t samples,uint32_t bitrate, uint32_t samplerate, uint32_t pad,uint8_t slot_size) {

	if (!bitrate || !samplerate) return 0;
    uint64_t num = (uint64_t)samples * (uint64_t)bitrate;   // e.g., 1152 * 128000
    uint64_t den = (uint64_t)8 * (uint64_t)samplerate;      // 8 bits/byte
    uint64_t base = num / den;
    uint64_t total = base + (pad ? slot_size : 0);
    return (total <= 0xFFFF) ? (uint16_t)total : 0;
}


void start_frame_info_machine(const char* file_name_in, const char* file_name_out,int dry_run){
	if(!file_name_in){

		fprintf(stderr,"Null input file name string!\n%s\n",strerror(errno));
		end_frame_info_machine(&machine);
		return;
	}
	else{
			
		fprintf(stdout,"Input file name string:\n%s\n",file_name_in);
		
	}
	if(!file_name_out){

		fprintf(stderr,"Null output file name string!\n%s\n",strerror(errno));
		end_frame_info_machine(&machine);
		return;
	}
	else{
			
		fprintf(stdout,"Output file name string:\n%s\n",file_name_out);
		
	}

	char path_buff_in[PATHSIZE*2]={0};
	char path_buff_out[PATHSIZE*10]={0};
	snprintf(path_buff_in,sizeof(path_buff_in)-1,"%s/%s",converter_in_dir,file_name_in);
	snprintf(path_buff_out,sizeof(path_buff_out)-1,"%s/%s%s",converter_out_dir,file_name_out,BOUNDARY_FILE_EXT);

	machine.fd_in=open(path_buff_in,O_RDONLY);
	if(machine.fd_in<0){

		fprintf(stderr,"Error at opening frame_info_machine input fd!: %s\nPath: %s\n",strerror(errno),path_buff_in);
		end_frame_info_machine(&machine);
		return;
	}
	machine.fd_out=(dry_run?0:open(path_buff_out,O_WRONLY|O_TRUNC|O_CREAT,0664));
	if(machine.fd_out<0){

		fprintf(stderr,"Error at opening frame_info_machine output fd!: %s\nPath: %s\n",strerror(errno),path_buff_out);
		end_frame_info_machine(&machine);
		return;
	}
	int nread=-1;
	uint64_t frame_id=0;
	int nwritten=-1;
	char hdr[MP3_SAMPLE_SIZE]={0};
	unsigned char id3[10];
	ssize_t r = read(machine.fd_in, id3, 10);
	if (r == 10 && id3[0]=='I' && id3[1]=='D' && id3[2]=='3') {
		size_t sz = ((id3[6]&0x7F)<<21)|((id3[7]&0x7F)<<14)|((id3[8]&0x7F)<<7)|(id3[9]&0x7F);
		int has_footer = (id3[5] & 0x10) != 0; // v2.4 footer flag
		lseek(machine.fd_in, 10 + sz + (has_footer ? 10 : 0), SEEK_SET);
	} else {
		lseek(machine.fd_in, 0, SEEK_SET);
	}
	while((nread=read(machine.fd_in,hdr,MP3_SAMPLE_SIZE))==4){
		 off_t start = lseek(machine.fd_in, 0, SEEK_CUR) - 4;  // where this header began

		// quick header sanity
		if ((unsigned char)hdr[0] != 0xFF ||
			(hdr[1] & 0xE0) != 0xE0 ||
			(hdr[1] & 0x18) == 0x08 ||
			(hdr[1] & 0x06) == 0x00 ||
			(hdr[2] & 0xF0) == 0xF0) {
			lseek(machine.fd_in, start + 1, SEEK_SET);
			continue;
		}
		// Data to be extracted from the header
		uint8_t   ver = (hdr[1] & 0x18) >> 3;   // Version index
		uint8_t   lyr = (hdr[1] & 0x06) >> 1;   // Layer index
		uint8_t   pad = (hdr[2] & 0x02) >> 1;   // Padding? 0/1
		uint8_t   brx = (hdr[2] & 0xf0) >> 4;   // Bitrate index
		uint8_t   srx = (hdr[2] & 0x0c) >> 2;   // SampRate index
		uint8_t   prot = (hdr[1] & 0x01); // 1=no CRC, 0=CRC present

		if (ver == 1 || brx == 0 || brx == 15 || srx > 2) {
			lseek(machine.fd_in, start + 1, SEEK_SET);
			continue;
		}
		uint32_t sample_rate = samplerate_table[ver][srx];
		uint32_t bitrate     = bitrate_table  [ver][lyr][brx] * 1000;
		uint16_t samples     = frame_samples_table[ver][lyr];   // 1152 or 576 for L3
		uint8_t  slotsize    = mpeg_slot_size[lyr];             // 1 for L3

		uint16_t size = get_mp3_frame_size(samples, bitrate, sample_rate, pad, slotsize);
		if (!size) { lseek(machine.fd_in, start + 1, SEEK_SET); continue; }
		if (!prot) size += 2;                 // add CRC if present
		if (size < 4) { lseek(machine.fd_in, start + 1, SEEK_SET); continue; }
		frame_info_t info = { frame_id++, (uint64_t)start, size, lyr, ver, brx, srx,
                      pad, samples, slotsize, sample_rate, bitrate };
		if (dry_run) print_frame_info_data(&info);
		else {
				if ((nwritten = write(machine.fd_out, &info, sizeof(info))) < 0) {
					fprintf(stderr,"We had issues with writing frame header info to file!\nAborting!\n%s\n",strerror(errno));
					end_frame_info_machine(&machine);
					return;
				}
		}
		// peek next header to confirm lock
		off_t next = (off_t)(start + size);
		lseek(machine.fd_in, next, SEEK_SET);
		unsigned char peek[4];
		if (read(machine.fd_in, peek, 4) == 4 &&
			peek[0] == 0xFF && (peek[1] & 0xE0) == 0xE0 &&
			(peek[1] & 0x18) != 0x08 && (peek[1] & 0x06) != 0x00)
		{
			lseek(machine.fd_in, next, SEEK_SET); // locked
		}
		else {
			// fallback: advance 1 byte from current frame start and try again
			lseek(machine.fd_in, start + 1, SEEK_SET);
		}
		memset(hdr,0,sizeof(hdr));
	}
	printf("Frame machine completed task!\n");
	end_frame_info_machine(&machine);
}

void print_frame_info_data(frame_info_t *frame_info){


	if(!frame_info){

		printf("Null frame info!!!\nWill not print\n");
		return;
	}

	printf("This frame header has the following data:\n"
				"frame_id: %lu\n"
				"mpeg_layer: %hu\n"
				"mpeg_version: %hu\n"
				"mpeg_bitrate_idx: %hu\n"
				"mpeg_sample_rate_idx: %hu\n"
				"bitrate: %u\n"
				"sample_rate: %u\n"
				"mpeg_padding: %hu\n"
				"mpeg_samples: %hu\n"
				"mpeg_slotsize: %hu\n"
				"frame_info_struct.start: %lu\n"
				"frame_info_struct.size: %hu\n\n\n",
				frame_info->frame_id,
				frame_info->mpeg_layer,
				frame_info->mpeg_version,
				frame_info->mpeg_bitrate_idx,
				frame_info->mpeg_sample_rate_idx,
				frame_info->bitrate,
				frame_info->sample_rate,
				frame_info->mpeg_padding,
				frame_info->mpeg_samples,
				frame_info->mpeg_slotsize,
				frame_info->start,
				frame_info->size
				);


}

void end_frame_info_machine(frame_info_machine_t* machine){

	printf("Exiting from frame machine\n");
	if(machine->fd_in){
		close(machine->fd_in);
	}
	if(machine->fd_out){
		close(machine->fd_out);
	}
}
