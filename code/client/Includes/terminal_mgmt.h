#ifndef TERMINAL_MGMT_H
#define TERMINAL_MGMT_H


#define TERMBUFFSIZE 1024
#define TERMIOS_BUFFER_THRESHOLD_BYTES 0
#define TERMIOS_INPUT_DELAY_TENTHS 10
#define TOTAL_NUM_TERM_FDS 3

#define S_TO_US(x) ((x*1000000))

#define US_TO_S(x) ((x/1000000))

#define MS_TO_US(x) ((x*1000))

#define US_TO_MS(x) ((x/1000))

#define F_S_TO_US(x) ((x*1000000.0))

#define F_US_TO_S(x) ((x/1000000.0))

#define F_MS_TO_US(x) ((x*1000.0))

#define F_US_TO_MS(x) ((x/1000.0))

#define UI_DEF_FRAMERATE_FPS 60.0

#define UI_DEF_FRAME_PERIOD_US ((uint64_t)(F_S_TO_US((1.0/UI_DEF_FRAMERATE_FPS))))

#define UI_FRAME_PERIOD_US(framer8) ((uint64_t)(F_S_TO_US((1.0/framer8))))
//Assumes numbers match OS specs.
//its crap.
//oh well

typedef enum {STDIN=0, STDOUT=1, STDERR=2}terminal_mgmt_fd;

typedef enum {TERMINAL_MGMT_CLEAR_SCREEN=0,
			TERMINAL_MGMT_MOVE_CURSOR_TO_POS=1,
			TERMINAL_MGMT_CLEAR_TO_END_OF_FILE=2,
			TERMINAL_MGMT_TOGGLE_CURSOR=3,
			TERMINAL_MGMT_RESET_COLOR=4,
			TERMINAL_MGMT_READ_FROM_FD=5,
			TERMINAL_MGMT_WRITE_TO_FD=6,
			TERMINAL_MGMT_REFRESH_SCREEN=7,
			TERMINAL_MGMT_INIT_STREAMS=8}terminal_mgmt_op;

void enable_raw(terminal_mgmt_fd fd);

void disable_raw(terminal_mgmt_fd fd);

int is_term_mgmt_raw(terminal_mgmt_fd fd);

void execute_terminal_op(terminal_mgmt_fd fd, terminal_mgmt_op op,int x, int y,char buff[],u_int64_t buff_size);

void enable_ncurses(void);

void endwin_wrapper(void);


#endif
