/*
#include "../player/SDL2/include/SDL.h"
#include "../player/SDL2/include/SDL_mixer.h"
#include "../player/SDL2/include/SDL_rwops.h"
*/
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

#define DEFAULT_CON_STRING "bring_it"
#define DEFAULT_PORT_STRING "bring_the_port"
#define DEFAULT_GOTIT_STRING "got_it"
#define DEFAULT_MORE_STRING "more"
#define DEFAULT_ENOUGH_STRING "enough"
#define DEFAULT_HOWLONG_STRING "howlong"
#define ACK_STRING "sure"
#define NACK_STRING "foff"
#define MUSIC_SERVER_INPUT_PATH "/songs_in/raw_songs/"
#define MAX_CLIENTS_HARD_LIMIT 1000
#define SIG_STOPSERVER 19199
#define PATHSIZE 20000
#define DEF_DATASIZE 1024
#define MAX_DATASIZE 100000
#define MAX_TRIES 10
#define BUFFSIZE 1024
