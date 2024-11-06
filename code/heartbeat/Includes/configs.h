#ifndef HB_CONFIGS_H
#define HB_CONFIGS_H

#define CONFIG_FILE_PATH_HB "./configs/sizes.cfg"
#define HB_ACK_TIMEOUT_LIM 8



extern int_pair hb_data_times_pair,
        hb_con_times_pair;


extern uint16_t hb_ack_timeout_lim;

void read_values_cfg_hb(void);
void print_values_cfg_hb(int fd);


#endif
