#ifndef HB_CONFIGS_H
#define HB_CONFIGS_H

#define CONFIG_FILE_PATH_HB "./configs/sizes.cfg"
#define HB_ACK_TIMEOUT_LIM 8

extern const uint8_t hb_display_splash;


extern ip_cache_entry heartbeat_ip_cache_entry,
		upper_ip_cache_entry;

extern char hb_server_name_buff[PATHSIZE+1];
extern int_pair hb_data_times_pair,
	hb_ack_times_pair,
        hb_con_times_pair;

extern uint64_t cfg_hb_ack_period_us;
extern uint16_t hb_ack_timeout_lim;

extern uint8_t cfg_hb_server_logging;

void read_values_cfg_hb(void);
void print_values_cfg_hb(int fd);


#endif
