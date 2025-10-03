#ifndef CONVERTER_H
#define CONVERTER_H


typedef struct frame_info_t{

	uint64_t start,end,size;


}frame_info_t;


typedef struct frame_info_machine{

	uint64_t n_frames;

	int fd_in,fd_out;


}frame_info_machine_t;



void start_frame_info_machine(const char* file_name_in);


void end_frame_info_machine(frame_info_machine_t* machine);

#endif
