#ifndef SOCKIO_TCP_H_H
#define SOCKIO_TCP_H_H
#include "sockio.h"


int sendsome(int sd,char buff[],size_t size,int_pair times);

int sendallfd(int sock,int fd,int_pair times,uint8_t is_ssl,SSL* cSSL);

int readsome(int sd,char buff[],size_t size,int_pair times);

int sendsome_ssl(SSL* ssl, const char* buf, size_t len, int_pair times);

int readsome_ssl(SSL* ssl, char* buf, size_t len, int_pair times);

int readalltofd(int sock,int fd,size_t down_size,int_pair times,uint8_t is_ssl,SSL* cSSL);


#endif
