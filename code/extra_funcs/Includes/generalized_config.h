#ifndef GENERALIZED_CONFIG_H
#define GENERALIZED_CONFIG_H

#define CONFIG_FILENAME "general_config.cfg"
extern int_pair port_mapper_times_pair;


//NULL terminated
void parse_generalized_cfg(char* dir_path);

void print_values_generalized_cfg(int fd);


#endif
