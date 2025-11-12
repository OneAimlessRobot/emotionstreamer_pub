#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include <ncurses.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <sys/ioctl.h> //for ioctl()
#include <linux/soundcard.h> //SOUND_PCM*
#include <alsa/asoundlib.h>
#include "../Includes/ripped_code.h"
#include "../Includes/terminal_mgmt.h"
#include "../../extra_funcs/Includes/fileshit.h"

static pthread_mutex_t close_ncurses_mtx=PTHREAD_MUTEX_INITIALIZER;

static int stream_arr_innited=0;

static int is_raw_fd[TOTAL_NUM_TERM_FDS]={0};

static int fd_mappings[TOTAL_NUM_TERM_FDS]={STDIN_FILENO,
					STDOUT_FILENO,
					STDERR_FILENO};
static FILE* fp_mappings[TOTAL_NUM_TERM_FDS]={NULL};

static int_pair fd_timepairs[TOTAL_NUM_TERM_FDS]={
						{0,500000},
						{0,500000},
						{0,500000}
						};

static int is_cursor_visible_fd[TOTAL_NUM_TERM_FDS]={0};

static struct termios orig_fd[TOTAL_NUM_TERM_FDS]={{0}};

static int putsome(int fd,char buff[],u_int64_t size,int_pair times){
                int iResult;
                struct timeval tv;
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(fd,&wfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(fd+1,(fd_set*)0,&wfds,(fd_set*)0,&tv);
                if(iResult>0){

                return write(fd,buff,size);
                }
                        else if(!iResult){
                return -2;
                }
                else{
                if(logging){

                fprintf(logstream, "SELECT ERROR!!!!! TERMINAL MGMT WRITE\n%s\n",strerror(errno));
                }
                return -1;
                }
}
static int getsome(int fd,char buff[],u_int64_t size,int_pair times){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(fd,&rfds);
                tv.tv_sec=times[0];
                tv.tv_usec=times[1];
                iResult=select(fd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){

                return read(fd,buff,size);
                }
                        else if(!iResult){
                return -2;
                }
                else{
                if(logging){

                fprintf(logstream, "SELECT ERROR!!!!! TERMINAL MGMT WRITE\n%s\n",strerror(errno));
                }
                return -1;
                }
}

void enable_raw(terminal_mgmt_fd fd) {

    struct termios raw;

    // get current terminal settings
    if (tcgetattr(fd_mappings[(int)fd], &orig_fd[fd]) == -1) {
        perror("tcgetattr out");
        exit(1);
    }
    int flags = fcntl(fd_mappings[(int)fd], F_GETFL, 0);
    fcntl(fd_mappings[(int)fd], F_SETFL, flags | O_NONBLOCK);
    
    raw = orig_fd[fd];

    // Input modes: no break, CR to NL, no parity check, no strip char,
    // no start/stop output control.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Output modes: disable post processing
    raw.c_oflag &= ~(OPOST);

    // Control modes: set 8-bit chars
    raw.c_cflag |= (CS8);

    // Local modes: echoing off, canonical off, no extended functions,
    // no signal chars (Ctrl-C, Ctrl-Z, etc.)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN|ISIG);

    // Control chars: return each byte, no timeout
    raw.c_cc[VMIN]  = TERMIOS_BUFFER_THRESHOLD_BYTES;
    raw.c_cc[VTIME] = TERMIOS_INPUT_DELAY_TENTHS;

    if (tcsetattr(fd_mappings[(int)fd], TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr in");
        exit(1);
    }
    is_raw_fd[fd_mappings[fd]]=1;
}

void disable_raw(terminal_mgmt_fd fd) {
    tcsetattr(fd_mappings[(int)fd], TCSAFLUSH, &orig_fd[fd]);
    is_raw_fd[fd]=0;
}
int is_term_mgmt_raw(terminal_mgmt_fd fd) {

	return is_raw_fd[fd];
}
void execute_terminal_op(terminal_mgmt_fd fd, terminal_mgmt_op op,int x,int y,char buff[],u_int64_t buff_size){

	switch(op){
		case TERMINAL_MGMT_CLEAR_SCREEN:
			dprintf(fd_mappings[(int)fd],"\033[2J");
		break;
		case TERMINAL_MGMT_REFRESH_SCREEN:
			if(stream_arr_innited){
				dprintf(fd_mappings[(int)fd],"\033[H");
    				fflush(fp_mappings[(int)fd]);
			}
		break;
		case TERMINAL_MGMT_INIT_STREAMS:
			if(!stream_arr_innited){
				fp_mappings[0]=stdin;
				fp_mappings[1]=stdout;
				fp_mappings[2]=stderr;
				stream_arr_innited=1;
			}
		break;
		case TERMINAL_MGMT_MOVE_CURSOR_TO_POS:
			dprintf(fd_mappings[(int)fd],"\033[%d;%dH", y, x);
		break;
		case TERMINAL_MGMT_CLEAR_TO_END_OF_FILE:
			dprintf(fd_mappings[(int)fd],"\033[K");
		break;
		case TERMINAL_MGMT_TOGGLE_CURSOR:
			if(is_cursor_visible_fd[fd]){
				is_cursor_visible_fd[fd]=0;
				dprintf(fd_mappings[(int)fd],"\033[?25h");
			}
			else{
				is_cursor_visible_fd[fd]=1;
				dprintf(fd_mappings[(int)fd],"\033[?25l");
			}
		break;
		case TERMINAL_MGMT_RESET_COLOR:
			dprintf(fd_mappings[(int)fd],"\033[0m");
		break;
		case TERMINAL_MGMT_READ_FROM_FD:
			getsome(fd_mappings[(int)fd], buff, buff_size,fd_timepairs[(int)fd]);
		break;
		case TERMINAL_MGMT_WRITE_TO_FD:
			putsome(fd_mappings[(int)fd], buff, buff_size,fd_timepairs[(int)fd]);
		break;
		default:
		break;

	}



}

void endwin_wrapper(void){
	pthread_mutex_lock(&close_ncurses_mtx);
	if(!isendwin()){

		print_string("Chamamos endwin wrapper!!!!!!!\n");
		endwin();
	}
	pthread_mutex_unlock(&close_ncurses_mtx);
}
void enable_ncurses(void){
    initscr();            // start ncurses
    cbreak();             // disable line buffering
    noecho();             // don't echo keypresses
    nodelay(stdscr, TRUE); // nonblocking input
    curs_set(0);          // hide cursor
    keypad(stdscr, TRUE); // enable arrow keys

}
/*
//clear entire screen
printf("\033[2J");        // clear entire screen
printf("\033[H");         // move cursor to home (row 1, col 1)
printf("\033[%d;%dH", y, x); // move cursor to (y, x)
printf("\033[K");         // clear to end of line

//hide/show cursor
printf("\033[?25l"); // hide
printf("\033[?25h"); // show

//colors
printf("\033[31m");   // red text
printf("\033[42m");   // green background
printf("\033[0m");    // reset
*/
