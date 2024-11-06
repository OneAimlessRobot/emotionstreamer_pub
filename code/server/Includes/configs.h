#ifndef CONFIGS__SERVER_H
#define CONFIGS__SERVER_H
#define TMP_CONFIG_FILE_PATH "./configs/.tmp_sizes.cfg"
#define CONFIG_FILE_PATH_SERVER "./configs/sizes.cfg"
#define MUSIC_SERVER_INPUT_PATH "/songs_in/raw_songs/"
#define SERVER_ACK_TIMEOUT_LIM 8
#define SERVER_CHUNK_SIZE 1024


extern char server_music_folder_path[PATHSIZE];

extern uint16_t server_chunk_size;

extern int_pair server_data_times_pair,
	server_con_times_pair,
	server_drop_chunks_times_pair;

extern uint16_t server_ack_timeout_lim;

void read_values_cfg_server(void);
void print_values_cfg_server(int fd);
void produce_config_file(void);



#endif
