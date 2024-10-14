#include "../../Includes/preprocessor.h"
#include "../Includes/fileshit.h"

FILE* logstream;

u_int64_t logging;

char curr_dir[PATHSIZE];

socklen_t socklenvar[1]= {sizeof(struct sockaddr)};
