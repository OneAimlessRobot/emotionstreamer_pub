#ifndef FILESHIT_H
#define FILESHIT_H
typedef uint16_t port_array[DEF_DATASIZE+1];
extern port_array attempted_port_arr;

extern FILE* logstream;

extern u_int8_t logging;

extern char curr_dir[PATHSIZE];

extern socklen_t socklenvar[2];

void _mkdir(const char *dir);

#endif
