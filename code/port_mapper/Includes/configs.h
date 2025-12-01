#ifndef CONFIGS_H
#define CONFIGS_H
#define CONFIG_FILE_PATH_PORT_MAPPER "./configs/config.cfg"

extern const uint8_t port_mapper_display_splash;

extern uint16_t cfg_num_ports,
		cfg_init_port;


extern ip_cache_entry port_mapper_ip_cache_entry;

extern char generalized_config_filepath_buff[PATHSIZE+1];


extern uint8_t cfg_port_mapper_logging;

void read_values_cfg_port_mapper(void);
void print_values_cfg_port_mapper(int fd);

#endif
 
