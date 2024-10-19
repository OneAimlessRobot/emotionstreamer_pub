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
#define CON_STRING "hey"
#define NACK_STRING "foff"
#define MUSIC_SERVER_INPUT_PATH "/songs_in/raw_songs/"
#define MAX_CLIENTS_HARD_LIMIT 1000
#define SIG_STOPSERVER 19199
#define DEF_DATASIZE 1024
#define PATHSIZE 1024
#define MAX_TRIES 10
