#ifndef FILESHIT_H
#define FILESHIT_H
typedef int port_array[DEF_DATASIZE];
extern port_array attempted_port_arr;
extern int num_attempted_ports;
extern char string_to_send[DEF_DATASIZE];

extern FILE* logstream;

extern u_int8_t logging;

extern char curr_dir[PATHSIZE];

extern socklen_t socklenvar[2];

void _mkdir(const char *dir);

#endif
