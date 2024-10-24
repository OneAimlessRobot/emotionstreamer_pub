#ifndef CONFIGS_H
#define CONFIGS_H
#define EXTENSION ".wav"
#define CONFIG_READ_LINE_BUFF_SIZE 1025

extern u_int64_t
        cfg_chunk_size,
        cfg_datasize;


void read_values_cfg_general(void);
void print_values_cfg_general(void);
#endif
