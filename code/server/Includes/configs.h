#ifndef CONFIGS__SERVER_H
#define CONFIGS__SERVER_H
#define TMP_CONFIG_FILE_PATH "./configs/.tmp_sizes.cfg"
#define CONFIG_FILE_PATH_SERVER "./configs/sizes.cfg"
#define MUSIC_SERVER_INPUT_PATH "/songs_in/raw_songs/"
#define SERVER_ACK_TIMEOUT_LIM 8
#define SERVER_CHUNK_SIZE 1024
#define WAV_MODE_EXTENSION ".wav"

extern char server_music_folder_path[PATHSIZE];
extern char server_working_extension[EXTENSION_SIZE];
extern ip_cache_entry server_ip_cache_entry;
extern ip_cache_entry upper_ip_cache_entry;
extern ip_cache_entry server_port_mapper_ip_cache_entry;
extern char generalized_config_filepath_buff[PATHSIZE+1];
extern char server_name_buff[PATHSIZE+1];

extern uint64_t server_chunk_size;
extern int8_t server_transmission_protocol;

extern int8_t is_wav_mode;

extern int_pair server_data_times_pair,
	server_con_times_pair,
	server_holepunching_times_pair,
	server_ack_times_pair,
	server_drop_chunks_times_pair;
extern uint64_t cfg_server_ack_period_us;

extern uint64_t server_ack_timeout_lim;

void read_values_cfg_server(void);
void print_values_cfg_server(int fd);
void produce_config_file(void);



#endif
