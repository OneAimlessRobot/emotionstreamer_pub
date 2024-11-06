#ifndef SOCK_OPS_H
#define SOCK_OPS_H
void setNonBlocking(int socket);

void print_sock_addr(int socket);

void init_addr(struct sockaddr_in* addr, char* hostname_str,uint16_t port);

int tryConnect(int* socket,int_pair times_pair,struct sockaddr_in* dst_addr);

#endif
