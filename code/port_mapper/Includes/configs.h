#ifndef CONFIGS_H
#define CONFIGS_H
#define CONFIG_FILE_PATH_PORT_MAPPER "./configs/config.cfg"


extern uint16_t cfg_num_ports,
		cfg_init_port;


extern ip_cache_entry port_mapper_ip_cache_entry;

extern char generalized_config_filepath_buff[PATHSIZE+1];



void read_values_cfg_port_mapper(void);
void print_values_cfg_port_mapper(int fd);

#endif
 
