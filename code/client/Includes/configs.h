#ifndef CONFIGS__CLIENT_H
#define CONFIGS__CLIENT_H
#define TYPE int32_t
#define SIZE sizeof(TYPE)
#define SAMPLE_MAX (pow(2,SIZE*8 - 1) - 1)

#define CHANNELS 2
#define SAMPLES 16384
#define DEVICE "default"
#define CONFIG_FILE_PATH_CLIENT "./configs/sizes.cfg"
#define LOG_FILE_NAME_CLIENT "./logs.log"
#define MUSIC_CLIENT_INPUT_PATH "./songs_out/raw_songs/"
#define CACHE_ALMOST_EMPTY_PCT 5
#define CACHE_ALMOST_FULL_PCT 60
#define CLIENT_ACK_TIMEOUT_LIM 8

extern ip_cache_entry server_ip_cache_entry,
		client_port_mapper_ip_cache_entry,
		client_ip_cache_entry;

extern u_int64_t
	cfg_latency_ms,
	cfg_freq,
	cfg_stream_decoder_cache_size_chunks,
	cfg_client_ack_timeout_lim,
	cfg_stream_player_cache_size_chunks;

extern uint8_t
	stream_enable_ncurses,
	stream_show_stats,
	stream_show_frames,
	cfg_cache_almost_full_pct,
	cfg_cache_almost_empty_pct,
	cfg_cache_almost_empty_pct;


extern uint16_t cfg_client_chunk_size;

extern float cfg_ui_framerate_fps;

extern uint64_t cfg_ui_frame_period_us;
extern int8_t streaming_protocol;
extern int8_t is_wav_mode;
extern char generalized_config_filepath_buff[PATHSIZE+1];
extern char server_ip_address_buff[PATHSIZE+1];
extern char client_ip_address_buff[PATHSIZE+1];
extern char client_music_folder_path[PATHSIZE];
extern char client_logs_file_name[PATHSIZE];
extern int_pair client_data_times_pair,
		client_con_times_pair,
		client_ack_times_pair,
		client_holepunching_times_pair;

extern uint64_t cfg_client_ack_period_us;
void read_values_cfg_client(void);
void print_values_cfg_client(int fd);



#endif
