#ifndef DOWNLOAD_FUNC_H
#define DOWNLOAD_FUNC_H

#define NUM_CHUNKS_TO_FILL_UP 1000

int downloadtofd(int sock,int fd,int64_t down_size, int_pair times, uint8_t is_ssl,SSL* cSSL);


#endif
