#ifndef GENERALIZED_CONFIG_H
#define GENERALIZED_CONFIG_H

#define CONFIG_FILENAME "general_config.cfg"

extern char port_remapper_ip_address[PATHSIZE];
extern ip_cache_entry port_mapper_entry;

extern int_pair port_mapper_times_pair;


//NULL terminated
void parse_generalized_cfg(char* dir_path);

void print_values_generalized_cfg(int fd);


#endif
