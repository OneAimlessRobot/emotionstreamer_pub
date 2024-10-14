#include "../../Includes/preprocessor.h"
#include "../Includes/sock_ops.h"
#include "../Includes/fileshit.h"
#include "../Includes/auxfuncs.h"




void setNonBlocking(int socket) {
     int flags = fcntl(socket, F_GETFL, 0);
     if (flags == -1) {
         fprintf(logstream,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno));
         return;
 }
     if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
         fprintf(logstream,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno)); 
    }
}

void print_sock_addr(int socket){
	
	struct sockaddr_in addr={0};

	getsockname(socket,(struct sockaddr*)(&addr),socklenvar);

	print_addr_aux("O endereço desta socket é:\n",&addr);

}
