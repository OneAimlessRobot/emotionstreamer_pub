#ifndef GENERALIZED_CONFIG_H
#define GENERALIZED_CONFIG_H

#define CONFIG_FILENAME "general_config.cfg"

#define S_TO_US(x) ((x*1000000))

#define US_TO_S(x) ((x/1000000))

#define MS_TO_US(x) ((x*1000))

#define US_TO_MS(x) ((x/1000))

#define F_S_TO_US(x) ((x*1000000.0))

#define F_US_TO_S(x) ((x/1000000.0))

#define F_MS_TO_US(x) ((x*1000.0))

#define F_US_TO_MS(x) ((x/1000.0))


extern int_pair port_mapper_times_pair;


//NULL terminated
void parse_generalized_cfg(char* dir_path);

void print_values_generalized_cfg(int fd);


#endif
