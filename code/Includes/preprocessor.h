#ifndef PRE_H
#define PRE_H
#define _GNU_SOURCE
#if defined(__ANDROID__) && defined(__linux__)
    #define EMSTREAM_ON_TERMUX 1
#else
    #define EMSTREAM_ON_TERMUX 0
#endif

#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <termios.h>
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
#include <stdatomic.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>


#define CONSTANT_TO_PRINT_TO_CLEAR_SCREEN_WITH_PRINTF "\033[3J\033[H\033[2J"
#define ACK_STRING "sup"
#define HB_SEND_STRING "imhere."
#define HB_REPLY_STRING "you never left. You never left."
#define CON_STRING "hey"
#define NACK_STRING "foff"
#define EXTENSION_SIZE 20
#define WAV_HEADER_SIZE 44
#define CONFIG_READ_LINE_BUFF_SIZE 1025
#define MAX_CLIENTS_HARD_LIMIT 1000
#define DEF_DATASIZE 1024
#define PATHSIZE 1024
#define MAX_TRIES 3
#endif
