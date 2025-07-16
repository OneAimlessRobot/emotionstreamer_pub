#ifndef BROWSER_CONFIGS_H
#define BROWSER_CONFIGS_H

#define CONFIG_FILE_PATH_BROWSER "./configs/sizes.cfg"

extern int_pair browser_data_times_pair,
	browser_holepunching_times_pair,
        browser_con_times_pair;

extern char generalized_config_filepath_buff[PATHSIZE+1];
void read_values_cfg_browser(void);
void print_values_cfg_browser(int fd);
#endif
