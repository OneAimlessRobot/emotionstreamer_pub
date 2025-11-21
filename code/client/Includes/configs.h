#ifndef CONFIGS__CLIENT_H
#define CONFIGS__CLIENT_H
#define TYPE int32_t
#define SIZE sizeof(TYPE)
#define SAMPLE_MAX (pow(2,SIZE*8 - 1) - 1)

#define CHANNELS 2
#define SAMPLES 16384
#define DEVICE "plughw:0,0"
#define CONFIG_FILE_PATH_CLIENT "./configs/sizes.cfg"
#define LOG_FILE_NAME_CLIENT "./logs.log"
#define MUSIC_CLIENT_INPUT_PATH "./songs_out/raw_songs/"
#define CACHE_ALMOST_EMPTY_PCT 5
#define CACHE_ALMOST_FULL_PCT 60
#define CLIENT_ACK_TIMEOUT_LIM 8
#define CLIENT_ALSA_LATENCY_MS_DEFAULT 200

extern ip_cache_entry server_ip_cache_entry,
		client_port_mapper_ip_cache_entry,
		client_ip_cache_entry;

extern u_int64_t
	cfg_client_alsa_device_latency_if_alsa_ms,
	cfg_ui_frame_period_us,
	cfg_latency_ms,
	cfg_freq,
	cfg_show_queue_length,
	cfg_show_decoder_queue_length,
	cfg_show_player_queue_length,
	cfg_stream_decoder_cache_size_chunks,
	cfg_stream_player_cache_size_chunks;

extern uint8_t
	cfg_client_logging,
	stream_enable_ncurses,
	stream_show_stats,
	stream_show_frames,
	cfg_cache_almost_full_pct,
	cfg_cache_almost_empty_pct,
	cfg_cache_almost_empty_pct,
	stream_show_decoder_queue,
	stream_show_player_queue;


extern uint16_t cfg_client_chunk_size;

extern float cfg_ui_framerate_fps;

extern int8_t is_wav_mode;
extern char generalized_config_filepath_buff[PATHSIZE+1],
	cfg_client_device_name_if_alsa[PATHSIZE+1],
	server_ip_address_buff[PATHSIZE+1],
	client_ip_address_buff[PATHSIZE+1],
	client_music_folder_path[PATHSIZE],
	client_logs_file_name[PATHSIZE];

extern int_pair client_data_times_pair,
		client_con_times_pair,
		client_ack_times_pair;

void read_values_cfg_client(void);
void print_values_cfg_client(int fd);



#endif
