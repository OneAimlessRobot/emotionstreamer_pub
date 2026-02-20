#ifndef SOCKIO_TCP_H_H
#define SOCKIO_TCP_H_H
#include "sockio.h"
int readall(int sock,char* buff,size_t size,int_pair times,uint8_t is_ssl,SSL* cSSL);

int sendall(int sock,char* buff,size_t size,int_pair times,uint8_t is_ssl,SSL* cSSL);

int sendsome(int sd,char buff[],size_t size,int_pair times,uint8_t is_ssl,SSL* cSSL);

int sendallfd(int sock,int fd,int_pair times,uint8_t is_ssl,SSL* cSSL);

int readsome(int sd,char buff[],size_t size,int_pair times,uint8_t is_ssl,SSL* cSSL);

int readalltofd(int sock,int fd,size_t down_size,int_pair times,uint8_t is_ssl,SSL* cSSL);

#endif
