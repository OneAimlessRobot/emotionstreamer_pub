#ifndef DOWNLOAD_FUNC_H
#define DOWNLOAD_FUNC_H

#define NUM_CHUNKS_TO_FILL_UP 1000

int downloadtofd(int sock,int fd,int down_size,int_pair times);


#endif
