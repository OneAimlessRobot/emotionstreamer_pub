#include "../../Includes/preprocessor.h"
#include "../Includes/fileshit.h"

FILE* logstream=NULL;

u_int8_t logging=0;

char curr_dir[PATHSIZE]={0};

socklen_t socklenvar[2]= {sizeof(struct sockaddr),sizeof(struct sockaddr_in)};
void _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/'){
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++){
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}
