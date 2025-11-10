#include "../../Includes/preprocessor.h"
#include "../Includes/terminal_mgmt.h"

#define TERMBUFFSIZE 1024

#define TERMIOS_BUFFER_THRESHOLD_BYTES 0
#define TERMIOS_INPUT_DELAY_TENTHS 10

static int is_raw_fd[TOTAL_NUM_TERM_FDS]={0};

static struct termios orig_fd[TOTAL_NUM_TERM_FDS]={{0}};


void enable_raw(terminal_mgmt_fd fd) {

    struct termios raw;

    // get current terminal settings
    if (tcgetattr((int)fd, &orig_fd[fd]) == -1) {
        perror("tcgetattr out");
        exit(1);
    }

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

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr in");
        exit(1);
    }
    is_raw_fd[fd]=1;
}

void disable_raw(terminal_mgmt_fd fd) {
    tcsetattr((int)fd, TCSAFLUSH, &orig_fd[fd]);
    is_raw_fd[fd]=0;
}
int is_raw(terminal_mgmt_fd fd) {

	return is_raw_fd[fd];
}
