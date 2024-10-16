#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
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
void init_addr(struct sockaddr_in* addr, char* hostname_str,uint32_t port){

         addr->sin_family=AF_INET;
         addr->sin_port= port;

         struct hostent* hp= gethostbyname(hostname_str);

         memcpy(&(addr->sin_addr),hp->h_addr,hp->h_length);


}

void tryConnect(int* socket,int_pair times_pair,struct sockaddr_in* dst_addr){
        int success=-1;
        int numOfTries=MAX_TRIES;


        while(success==-1&& numOfTries){
                printf("Tentando conectar a %s (Tentativa %d)!!!!!!\n",inet_ntoa(dst_addr->sin_addr),-numOfTries+MAX_TRIES+1);
                success=connect(*socket,(struct sockaddr*)dst_addr,*socklenvar);
               int sockerr;
                getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,socklenvar);
                numOfTries--;
                if(sockerr==EINPROGRESS){

                        fprintf(stderr,"Erro normal:%s\n Erro Socket%s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
                        continue;

                }
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET((*socket),&wfds);

                struct timeval t;
                t.tv_sec=times_pair[0];
                t.tv_usec=times_pair[1];
                int iResult=select((*socket)+1,0,&wfds,0,&t);

                if(iResult>0&&!success&&((*socket)!=-1)){
                //printf("Coneccao de %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
                        break;

                }
                fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
        }
        if(!numOfTries){
        printf("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAX_TRIES);
        raise(SIGINT);
        }

}

