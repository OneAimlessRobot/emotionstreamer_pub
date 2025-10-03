#ifndef CONFIGS_H
#define CONFIGS_H

#define CONFIG_FILE_PATH_CONVERTER "./configs/config.cfg"


extern char converter_in_dir[PATHSIZE];

extern char converter_out_dir[PATHSIZE];


void read_values_cfg_converter(void);
void print_values_cfg_converter(int fd);


#endif
