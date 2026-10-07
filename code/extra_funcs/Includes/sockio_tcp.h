#ifndef SOCKIO_TCP_H_H
#define SOCKIO_TCP_H_H
#include "sockio.h"


int64_t sendsome(int sd,char buff[],int64_t size,int_pair times, int flags);

int sendallfd(int sock,int fd,int_pair times,uint8_t is_ssl,SSL* cSSL);

int64_t readsome(int sd,char buff[],int64_t size,int_pair times, int flags);

int64_t sendsome_ssl(SSL* ssl, const char* buf, int64_t len, int_pair times);

int64_t readsome_ssl(SSL* ssl, char* buf, int64_t len, int_pair times);

int readalltofd(int sock,int fd,int64_t down_size,int_pair times,uint8_t is_ssl,SSL* cSSL);

int64_t sendall(int socket,SSL*ctx_if_ssl,char* buff, int64_t total_to_send, uint8_t is_tls,int_pair times);

int64_t readall(int socket,SSL*ctx_if_ssl,char* buff, int64_t total_to_read, uint8_t is_tls, int_pair times);

#endif
