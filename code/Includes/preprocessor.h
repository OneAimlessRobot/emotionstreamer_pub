#ifndef PRE_H
#define PRE_H
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ACK_STRING "sup"
#define HB_SEND_STRING "imhere."
#define HB_REPLY_STRING "you never left. You never left."
#define MASTER_SEND_STRING "imhere."
#define MASTER_REPLY_STRING "you never left. You never left."
#define CON_STRING "hey"
#define NACK_STRING "foff"
#define EXTENSION ".wav"
#define CONFIG_READ_LINE_BUFF_SIZE 1025
#define MAX_CLIENTS_HARD_LIMIT 1000
#define DEF_DATASIZE 1024
#define PATHSIZE 1024
#define MAX_TRIES 2
#endif
