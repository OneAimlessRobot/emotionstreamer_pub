 #ifndef MASTER_CONFIGS_H
#define MASTER_CONFIGS_H

#define CONFIG_FILE_PATH_MASTER "./configs/sizes.cfg"
#define MASTER_ACK_TIMEOUT_LIM 8



extern int_pair master_data_times_pair,
        master_con_times_pair;


extern uint16_t master_ack_timeout_lim;

void read_values_cfg_master(void);
void print_values_cfg_master(int fd);


#endif
