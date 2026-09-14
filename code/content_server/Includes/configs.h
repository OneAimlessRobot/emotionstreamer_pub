#ifndef CONFIGS__SERVER_H
#define CONFIGS__SERVER_H
#define TMP_CONFIG_FILE_PATH "./configs/.tmp_sizes.cfg"
#define TMP_ROTATION_FILE_PATH "./configs/.tmp_rotation.cfg"
#define CONFIG_FILE_PATH_SERVER "./configs/sizes.cfg"
#define ROTATION_FILE_PATH_SERVER "./configs/rotation.ini"
#define ROTATION_LENGTH_LIMIT 100
#define DEFAULT_ROTATION_PERIOD 100
#define ROTATION_SONG_FILENAME_LENGTH (PATHSIZE)
#define MUSIC_SERVER_INPUT_PATH "/raw_songs/"
#define MUSIC_SERVER_QUARANTINE_PATH "/quarantine_songs/"
#define SERVER_ACK_TIMEOUT_LIM 8
#define SERVER_CHUNK_SIZE 1024
#define WAV_MODE_EXTENSION ".wav"

extern const uint8_t server_display_splash;

extern char server_music_folder_path[PATHSIZE+1],
	server_music_quarantine_folder_path[PATHSIZE+1],
	* server_tmp_dir_path,
	server_working_extension[EXTENSION_SIZE],
	server_name_buff[PATHSIZE+1],
	curr_server_quarantine_dir_buff[PATHSIZE+1],
	server_auto_mode_rotation[ROTATION_LENGTH_LIMIT][ROTATION_SONG_FILENAME_LENGTH],
	server_auto_mode_rotation_filename[CONFIG_READ_LINE_BUFF_SIZE],
	content_server_ip_address[PATHSIZE+1];


extern ip_cache_entry upper_ip_cache_entry;


extern struct timeval rotation_period,
                	curr_song_waited_time;


extern int_pair server_data_times_pair,
	server_con_times_pair,
	server_ack_times_pair,
	server_drop_chunks_times_pair;



/*

	ints

*/

extern unsigned int is_auto_mode,
		curr_num_songs_rotation,
		curr_song_index_rotation;

extern int child_pid;




extern uint64_t cfg_server_ack_period_us,
		server_ack_timeout_lim,
		server_chunk_size,
		curr_request_id;

extern uint16_t is_wav_mode;

extern uint8_t
	cfg_server_print_config,
	cfg_server_show_splash,
	cfg_server_logging,
	cfg_server_slave_mode;


void read_values_cfg_server(void);
void print_values_cfg_server(int fd);
void produce_config_file(void);
void produce_rotation_file(void);



#endif
