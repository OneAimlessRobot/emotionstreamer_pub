#ifndef AUXFUNCS_H
#define AUXFUNCS_H
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

#define LOGO_ASCII_ART_FILE_PATH "../config/logo.txt"

#define LOGO_ASCII_ART_FILE_SLEEP_PRINT_TIME_MS 100
/*
credits for stdout colors:


https://gist.github.com/radxene/f1e286301763b921baf06074ea46c800


*/

#define ANSI_RESET_ALL          "\x1b[0m"

#define ANSI_COLOR_BLACK        "\x1b[30m"
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_WHITE        "\x1b[37m"

#define ANSI_BACKGROUND_BLACK   "\x1b[40m"
#define ANSI_BACKGROUND_RED     "\x1b[41m"
#define ANSI_BACKGROUND_GREEN   "\x1b[42m"
#define ANSI_BACKGROUND_YELLOW  "\x1b[43m"
#define ANSI_BACKGROUND_BLUE    "\x1b[44m"
#define ANSI_BACKGROUND_MAGENTA "\x1b[45m"
#define ANSI_BACKGROUND_CYAN    "\x1b[46m"
#define ANSI_BACKGROUND_WHITE   "\x1b[47m"

#define ANSI_STYLE_BOLD         "\x1b[1m"
#define ANSI_STYLE_ITALIC       "\x1b[3m"
#define ANSI_STYLE_UNDERLINE    "\x1b[4m"



typedef enum var_op{V_LOOK,V_SET}var_op;

void print_out_logo(void);

int acess_var_mtx(pthread_mutex_t* mtx,int* var,int value_if_change,var_op op);

uint16_t acess_var_mtx_uint16(pthread_mutex_t* mtx,uint16_t* var,uint16_t value_if_change,var_op op);

//All strings are null terminated
int strs_are_strictly_equal(char* str1, char* str2);

int min(int first,int second);

int max(int first,int second);

void swap(void* first,void* second,int sz_bytes);

void print_addr_aux(char* prompt,struct sockaddr_in* addr);

int does_dir_exist_aux(char* path);

int randInteger(int min, int max);

char* randStr(int size, char* buff);

void snprint_addr_aux(char* dst,uint16_t* port,uint32_t size,struct sockaddr_in* addr);

int proto_is_tcp(int proto);
void print_current_date(void);
//these assume the timeval structs are not null!!!
void time_spec_sum_function(struct timeval* time_one,struct timeval* time_two,struct timeval* time_out);
//we assume time two is bigger!!!
//it is two  minus one!
//order matters!
void time_spec_sub_function(struct timeval* time_one,struct timeval* time_two,struct timeval* time_out);
//time_one bigger (1), smaller(-1) or equal (0) to time_two?
int time_spec_compare_function(struct timeval* time_one,struct timeval* time_two);
void time_spec_print_function(int fd, const char* timeval_name, struct timeval* time_printed);

void skip_config_comments(FILE* fp);
#endif
