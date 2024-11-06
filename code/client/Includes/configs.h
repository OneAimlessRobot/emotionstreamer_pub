#ifndef CONFIGS__CLIENT_H
#define CONFIGS__CLIENT_H
#define TYPE int16_t
#define SIZE sizeof(TYPE)
#define SAMPLE_MAX (pow(2,SIZE*8 - 1) - 1)
#define CHANNELS 2
#define DEVICE "default"
#define CONFIG_FILE_PATH_CLIENT "./configs/sizes.cfg"
#define LOG_FILE_NAME_CLIENT "./logs.log"
#define MUSIC_CLIENT_INPUT_PATH "./songs_out/raw_songs/"
#define CACHE_ALMOST_EMPTY_PCT 5
#define CACHE_ALMOST_FULL_PCT 60
#define CLIENT_ACK_TIMEOUT_LIM 8


extern u_int64_t
	cfg_latency_ms,
	cfg_freq,
	cfg_stream_cache_size_chunks;

extern uint16_t
	cfg_cache_almost_full_pct,
	cfg_cache_almost_empty_pct,
	cfg_client_ack_timeout_lim,
	cfg_cache_almost_empty_pct;


extern char client_music_folder_path[PATHSIZE];
extern char client_logs_file_name[PATHSIZE];
extern int_pair client_data_times_pair;
extern int_pair client_con_times_pair;


void read_values_cfg_client(void);
void print_values_cfg_client(int fd);



#endif
