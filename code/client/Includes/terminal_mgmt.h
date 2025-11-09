#ifndef TERMINAL_MGMT_H
#define TERMINAL_MGMT_H


#define TERMBUFFSIZE 1024
#define TERMIOS_BUFFER_THRESHOLD_BYTES 0
#define TERMIOS_INPUT_DELAY_TENTHS 0
#define TOTAL_NUM_TERM_FDS 3
//Assumes numbers match OS specs.
//its crap.
//oh well

typedef enum {STDIN=0, STDOUT=1, STDERR=2}terminal_mgmt_fd;

void enable_raw(terminal_mgmt_fd fd);

void disable_raw(terminal_mgmt_fd fd);

int is_raw(terminal_mgmt_fd fd);

#endif
