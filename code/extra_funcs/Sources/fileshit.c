#include "../../Includes/preprocessor.h"
#include <openssl/ssl.h>
#include "../Includes/fileshit.h"

FILE* logstream=NULL;
port_array attempted_port_arr={0};

u_int8_t logging=0;

u_int8_t will_use_tls=0;


char auth_cert_file_path[PATHSIZE]={0};
char host_cert_file_path[PATHSIZE]={0};
char host_cert_pkey_file_path[PATHSIZE]={0};

char curr_dir[PATHSIZE]={0};

SSL_CTX *global_ctx=NULL;

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
