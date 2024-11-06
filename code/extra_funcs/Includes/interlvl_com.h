#ifndef INTERLVL_COM_H
#define INTERLVL_COM_H

typedef enum{SERVER,HB_SERVER,M_SERVER,TYPE_NA}module_type;
//NULL TERMI8ed
module_type string_to_module_type(char*str);

//Nuff space (At least PATHSIZE)
void module_type_to_string(module_type type, char* buff);

typedef struct slave_args{

	char* conf_str,*send_str,*reply_str,
		*lower_name;
	struct sockaddr_in this_addr,
		master_addr;
	uint16_t exit_signal;
	uint16_t ack_timeout_lim;
	uint32_t sleep_us;
	int* start_trigger;
	int* loop_var;
	int_pair con_times_pair;
	int_pair data_times_pair;
	pthread_mutex_t* var_mtx;
	pthread_cond_t* trg_cond;
	module_type type;
	
}slave_args;


typedef struct con_set{

	int max_size;
	int curr_size;
	int*fd_arr;
	int*timeout_arr;
	con_t* con_arr;
        fd_set rdfds;
	pthread_cond_t* start_cond;
	pthread_mutex_t* set_mtx;
}con_set;
typedef struct overseer_args{

	int*is_on;
	int exit_signal;
	pthread_mutex_t* start_cond_mtx;
	pthread_mutex_t* var_mtx;
	struct con_set* cons;
	char* send_str,*reply_str;
	int_pair data_times_pair;
	uint16_t ack_timeout_lim;



}overseer_args;


void* watch_dog_func(void* args);
void init_con_set(con_set* set,con_t* con_buff,int* timeout_buff,int* fd_buff,int max_size,pthread_mutex_t* mtx,pthread_cond_t* cond);
void close_all_fds(con_set* set);
void add_con(con_set* set,con_t*con,char* buff);
void kill_con(con_set* set,int index);
void* slave_thread(void* args);
void init_module_tcp_stuff(int* sockptr,char* addr,uint16_t tcp_s_port,struct sockaddr_in * sockaddr_buff,int exit_signal,int max_connected);
#endif
