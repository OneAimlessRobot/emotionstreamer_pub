#ifndef BROWSER_CONFIGS_H
#define BROWSER_CONFIGS_H

#define CONFIG_FILE_PATH_BROWSER "./configs/sizes.cfg"

extern const uint8_t browser_display_splash;

extern int_pair browser_data_times_pair,
		browser_ack_times_pair,
        	browser_con_times_pair;

extern uint64_t cfg_browser_ack_period_us;


extern uint8_t cfg_server_browser_logging;

extern ip_cache_entry server_browser_port_mapper_ip_cache_entry;
extern ip_cache_entry server_browser_ip_cache_entry;
extern char generalized_config_filepath_buff[PATHSIZE+1];
void read_values_cfg_browser(void);
void print_values_cfg_browser(int fd);
#endif
