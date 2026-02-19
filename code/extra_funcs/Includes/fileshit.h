#ifndef FILESHIT_H
#define FILESHIT_H
typedef uint16_t port_array[DEF_DATASIZE+1];
extern port_array attempted_port_arr;

extern FILE* logstream;

extern SSL_CTX *global_ctx;

extern u_int8_t logging;

extern u_int8_t will_use_tls;


extern char curr_dir[PATHSIZE];

extern char auth_cert_file_path[PATHSIZE];

extern char host_cert_file_path[PATHSIZE];

extern char host_pkey_file_path[PATHSIZE];

extern socklen_t socklenvar[2];

void _mkdir(const char *dir);

#endif
