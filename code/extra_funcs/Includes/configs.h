#ifndef CONFIGS_H
#define CONFIGS_H

#define CONFIG_READ_LINE_BUFF_SIZE 1025

extern u_int64_t cfg_chunk_size,
	cfg_freq,
	cfg_datasize,
	cfg_stream_cache_size_chunks;


extern char server_music_folder_path[PATHSIZE];
extern int_pair server_data_times_pair;
extern int_pair client_data_times_pair;
extern int_pair client_con_times_pair;
extern int_pair server_con_times_pair;
extern int_pair server_drop_chunks_times_pair;


void read_values_cfg(void);
void print_values_cfg(void);



#endif
