#ifndef INTERLVL_COM_H
#define INTERLVL_COM_H

typedef enum{SERVER,HB_SERVER,M_SERVER,TYPE_NA}module_type;
typedef typeof(void (int))  *quit_func;
typedef typeof(void (void))  *cleanup_func;

//NULL TERMI8ed
module_type string_to_module_type(char*str);

//Nuff space (At least PATHSIZE)
void module_type_to_string(module_type type, char* buff);

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


typedef struct slave_args{

	char* conf_str,
		*lower_name;
	struct sockaddr_in this_addr,
		this_con_addr,
		master_addr;
	uint16_t exit_signal;
	quit_func sig_func;
	cleanup_func clean_func;
	uint64_t ack_timeout_lim;
	uint64_t sleep_us;
	atomic_int* start_trigger;
	con_t* con_obj;
	int8_t heartbeat_protocol,
		transmit_protocol;
	atomic_int* loop_var;
	int_pair con_times_pair;
	int_pair data_times_pair;
	int_pair ack_times_pair;
	uint64_t ack_period_us;
	int_pair holepunching_times_pair;
	pthread_mutex_t* var_mtx;
	pthread_mutex_t* con_mtx;
	pthread_cond_t* trg_cond;
	module_type type;
	char* extension_buff;
	ip_cache_entry slave_port_mapper_ip_cache_entry;
	ip_cache_entry slave_ip_cache_entry;

}slave_args;


typedef struct overseer_args{

	atomic_int* is_on;
	int exit_signal;
	quit_func sig_func;
	cleanup_func clean_func;
	pthread_mutex_t* start_cond_mtx;
	pthread_mutex_t* var_mtx;
	struct con_set* cons;
	int8_t heartbeat_protocol;
	int_pair data_times_pair;
	int_pair holepunching_times_pair;
	int_pair ack_times_pair;
	uint64_t ack_period_us;
	uint64_t ack_timeout_lim;



}overseer_args;

typedef struct acceptor_args{

	atomic_int* is_on;
	atomic_int* started;
	int exit_signal;
	quit_func sig_func;
	cleanup_func clean_func;
	int_pair data_times_pair;
	int_pair con_times_pair;
	int_pair ack_times_pair;
	uint64_t ack_period_us;
	int_pair holepunching_times_pair;
	int8_t heartbeat_protocol;
	fd_set mainfds;
        int accept_sockfd;
        struct sockaddr_in accept_addr;
	pthread_mutex_t * master_mtx;
	pthread_mutex_t * var_mtx;
	pthread_mutex_t * con_mtx;
	struct overseer_args* arg_o;
	struct slave_args* arg_s;
	ip_cache_entry acceptor_port_mapper_ip_cache_entry;

}acceptor_args;

void init_con_set(con_set* set,con_t* con_buff,int* timeout_buff,int* fd_buff,int max_size,pthread_mutex_t* mtx,pthread_cond_t* cond);
void close_all_fds(con_set* set);
void add_con(con_set* set,con_t*con,char* type_buff,int id,char* name_buff,char* ip_buff,uint16_t stored_port,char* extension_buff,char* transmit_proto_buff,char* heartbeat_proto_buff);
void init_module_tcp_stuff(int* sockptr,char* addr,uint16_t tcp_s_port,struct sockaddr_in * sockaddr_buff,int exit_signal,int max_connected,int is_port_mapper,ip_cache_entry* port_mapper_cache_entry);


void* slave_thread(void* args);
void* watch_dog_func(void* args);
void* acceptor_func(void* args);


#endif
