#ifndef FILESHIT_H
#define FILESHIT_H

#define PROTO_ARR_NUM_CELLS 4

#define PROTO_ARR_SIZE (PROTO_ARR_NUM_CELLS* sizeof(uint16_t))



typedef uint16_t proto_arr[PROTO_ARR_NUM_CELLS];

typedef uint16_t port_array[DEF_DATASIZE+1];


extern port_array attempted_port_arr;

typedef typeof(void (void)) *exit_func_t;

extern FILE* logstream;

extern SSL_CTX *global_server_ctx,
		*global_client_ctx;

extern u_int8_t logging;

extern u_int8_t use_exit_func;

extern u_int16_t will_use_tls;



extern char curr_dir[PATHSIZE];

extern char auth_cert_file_path[PATHSIZE];

extern char host_cert_file_path[PATHSIZE];

extern char host_pkey_file_path[PATHSIZE];

extern socklen_t socklenvar[2];

void _mkdir(const char *dir);


extern exit_func_t exit_func_for_this_module;

#endif
