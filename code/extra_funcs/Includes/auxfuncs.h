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

typedef enum var_op{V_LOOK,V_SET}var_op;

int acess_var_mtx(pthread_mutex_t* mtx,int* var,int value_if_change,var_op op);

uint16_t acess_var_mtx_uint16(pthread_mutex_t* mtx,uint16_t* var,uint16_t value_if_change,var_op op);

//All strings are null terminated
int strs_are_strictly_equal(char* str1, char* str2);

int min(int first,int second);

int max(int first,int second);

void swap(void* first,void* second,int sz_bytes);

void print_addr_aux(char* prompt,struct sockaddr_in* addr);
int randInteger(int min, int max);
char* randStr(int size, char* buff);
void snprint_addr_aux(char* dst,uint32_t size,struct sockaddr_in* addr);
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

#endif
